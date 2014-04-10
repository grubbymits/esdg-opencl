#include "LE1MachineScheduler.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetInstrInfo.h"

using namespace llvm;

void LE1SchedStrategy::SchedRemainder::
init(ScheduleDAGMI *DAG, const TargetSchedModel *SchedModel) {
  reset();
  if (!SchedModel->hasInstrSchedModel())
    return;
  RemainingCounts.resize(SchedModel->getNumProcResourceKinds());
  for (std::vector<SUnit>::iterator
         I = DAG->SUnits.begin(), E = DAG->SUnits.end(); I != E; ++I) {
    const MCSchedClassDesc *SC = DAG->getSchedClass(&*I);
    RemIssueCount += SchedModel->getNumMicroOps(I->getInstr(), SC)
      * SchedModel->getMicroOpFactor();
    for (TargetSchedModel::ProcResIter
           PI = SchedModel->getWriteProcResBegin(SC),
           PE = SchedModel->getWriteProcResEnd(SC); PI != PE; ++PI) {
      unsigned PIdx = PI->ProcResourceIdx;
      unsigned Factor = SchedModel->getResourceFactor(PIdx);
      RemainingCounts[PIdx] += (Factor * PI->Cycles);
    }
  }
}

void LE1SchedStrategy::SchedBoundary::
init(ScheduleDAGMI *dag, const TargetSchedModel *smodel, SchedRemainder *rem) {
  reset();
  DAG = dag;
  SchedModel = smodel;
  Rem = rem;
  if (SchedModel->hasInstrSchedModel())
    ExecutedResCounts.resize(SchedModel->getNumProcResourceKinds());
}

/// Initialize the per-region scheduling policy.
void LE1SchedStrategy::initPolicy(MachineBasicBlock::iterator Begin,
                                     MachineBasicBlock::iterator End,
                                     unsigned NumRegionInstrs) {
  const TargetMachine &TM = Context->MF->getTarget();

  // Avoid setting up the register pressure tracker for small regions to save
  // compile time. As a rough heuristic, only track pressure when the number of
  // schedulable instructions exceeds half the integer register file.
  unsigned NIntRegs = Context->RegClassInfo->getNumAllocatableRegs(
    TM.getTargetLowering()->getRegClassFor(MVT::i32));

  RegionPolicy.ShouldTrackPressure = NumRegionInstrs > (NIntRegs / 2);

  // For generic targets, we default to bottom-up, because it's simpler and more
  // compile-time optimizations have been implemented in that direction.
  RegionPolicy.OnlyBottomUp = true;

  // Allow the subtarget to override default policy.
  const TargetSubtargetInfo &ST = TM.getSubtarget<TargetSubtargetInfo>();
  ST.overrideSchedPolicy(RegionPolicy, Begin, End, NumRegionInstrs);

  // After subtarget overrides, apply command line options.
  //if (!EnableRegPressure)
    RegionPolicy.ShouldTrackPressure = false;

  // Check -misched-topdown/bottomup can force or unforce scheduling direction.
  // e.g. -misched-bottomup=false allows scheduling in both directions.
    /*
  assert((!ForceTopDown || !ForceBottomUp) &&
         "-misched-topdown incompatible with -misched-bottomup");
  if (ForceBottomUp.getNumOccurrences() > 0) {
    RegionPolicy.OnlyBottomUp = ForceBottomUp;
    if (RegionPolicy.OnlyBottomUp)
      RegionPolicy.OnlyTopDown = false;
  }
  if (ForceTopDown.getNumOccurrences() > 0) {
    RegionPolicy.OnlyTopDown = ForceTopDown;
    if (RegionPolicy.OnlyTopDown)
      RegionPolicy.OnlyBottomUp = false;
  }
  */
}

void LE1SchedStrategy::initialize(ScheduleDAGMI *dag) {
  DAG = dag;
  SchedModel = DAG->getSchedModel();
  TRI = DAG->TRI;

  Rem.init(DAG, SchedModel);
  Top.init(DAG, SchedModel, &Rem);
  Bot.init(DAG, SchedModel, &Rem);

  // Initialize resource counts.

  // Initialize the HazardRecognizers. If itineraries don't exist, are empty, or
  // are disabled, then these HazardRecs will be disabled.
  const InstrItineraryData *Itin = SchedModel->getInstrItineraries();
  const TargetMachine &TM = DAG->MF.getTarget();
  if (!Top.HazardRec) {
    Top.HazardRec =
      TM.getInstrInfo()->CreateTargetMIHazardRecognizer(Itin, DAG);
  }
  if (!Bot.HazardRec) {
    Bot.HazardRec =
      TM.getInstrInfo()->CreateTargetMIHazardRecognizer(Itin, DAG);
  }
}

void LE1SchedStrategy::releaseTopNode(SUnit *SU) {
  if (SU->isScheduled)
    return;

  for (SUnit::pred_iterator I = SU->Preds.begin(), E = SU->Preds.end();
       I != E; ++I) {
    if (I->isWeak())
      continue;
    unsigned PredReadyCycle = I->getSUnit()->TopReadyCycle;
    unsigned Latency = I->getLatency();
#ifndef NDEBUG
    Top.MaxObservedLatency = std::max(Latency, Top.MaxObservedLatency);
#endif
    if (SU->TopReadyCycle < PredReadyCycle + Latency)
      SU->TopReadyCycle = PredReadyCycle + Latency;
  }
  Top.releaseNode(SU, SU->TopReadyCycle);
}

void LE1SchedStrategy::releaseBottomNode(SUnit *SU) {
  if (SU->isScheduled)
    return;

  assert(SU->getInstr() && "Scheduled SUnit must have instr");

  for (SUnit::succ_iterator I = SU->Succs.begin(), E = SU->Succs.end();
       I != E; ++I) {
    if (I->isWeak())
      continue;
    unsigned SuccReadyCycle = I->getSUnit()->BotReadyCycle;
    unsigned Latency = I->getLatency();
#ifndef NDEBUG
    Bot.MaxObservedLatency = std::max(Latency, Bot.MaxObservedLatency);
#endif
    if (SU->BotReadyCycle < SuccReadyCycle + Latency)
      SU->BotReadyCycle = SuccReadyCycle + Latency;
  }
  Bot.releaseNode(SU, SU->BotReadyCycle);
}

/// Set IsAcyclicLatencyLimited if the acyclic path is longer than the cyclic
/// critical path by more cycles than it takes to drain the instruction buffer.
/// We estimate an upper bounds on in-flight instructions as:
///
/// CyclesPerIteration = max( CyclicPath, Loop-Resource-Height )
/// InFlightIterations = AcyclicPath / CyclesPerIteration
/// InFlightResources = InFlightIterations * LoopResources
///
/// TODO: Check execution resources in addition to IssueCount.
void LE1SchedStrategy::checkAcyclicLatency() {
  if (Rem.CyclicCritPath == 0 || Rem.CyclicCritPath >= Rem.CriticalPath)
    return;

  // Scaled number of cycles per loop iteration.
  unsigned IterCount =
    std::max(Rem.CyclicCritPath * SchedModel->getLatencyFactor(),
             Rem.RemIssueCount);
  // Scaled acyclic critical path.
  unsigned AcyclicCount = Rem.CriticalPath * SchedModel->getLatencyFactor();
  // InFlightCount = (AcyclicPath / IterCycles) * InstrPerLoop
  unsigned InFlightCount =
    (AcyclicCount * Rem.RemIssueCount + IterCount-1) / IterCount;
  unsigned BufferLimit =
    SchedModel->getMicroOpBufferSize() * SchedModel->getMicroOpFactor();

  Rem.IsAcyclicLatencyLimited = InFlightCount > BufferLimit;

  DEBUG(dbgs() << "IssueCycles="
        << Rem.RemIssueCount / SchedModel->getLatencyFactor() << "c "
        << "IterCycles=" << IterCount / SchedModel->getLatencyFactor()
        << "c NumIters=" << (AcyclicCount + IterCount-1) / IterCount
        << " InFlight=" << InFlightCount / SchedModel->getMicroOpFactor()
        << "m BufferLim=" << SchedModel->getMicroOpBufferSize() << "m\n";
        if (Rem.IsAcyclicLatencyLimited)
          dbgs() << "  ACYCLIC LATENCY LIMIT\n");
}

void LE1SchedStrategy::registerRoots() {
  Rem.CriticalPath = DAG->ExitSU.getDepth();

  // Some roots may not feed into ExitSU. Check all of them in case.
  for (std::vector<SUnit*>::const_iterator
         I = Bot.Available.begin(), E = Bot.Available.end(); I != E; ++I) {
    if ((*I)->getDepth() > Rem.CriticalPath)
      Rem.CriticalPath = (*I)->getDepth();
  }
  DEBUG(dbgs() << "Critical Path: " << Rem.CriticalPath << '\n');

  /*
  if (EnableCyclicPath) {
    Rem.CyclicCritPath = DAG->computeCyclicCriticalPath();
    checkAcyclicLatency();
  }
  */
}

/// Does this SU have a hazard within the current instruction group.
///
/// The scheduler supports two modes of hazard recognition. The first is the
/// ScheduleHazardRecognizer API. It is a fully general hazard recognizer that
/// supports highly complicated in-order reservation tables
/// (ScoreboardHazardRecognizer) and arbitraty target-specific logic.
///
/// The second is a streamlined mechanism that checks for hazards based on
/// simple counters that the scheduler itself maintains. It explicitly checks
/// for instruction dispatch limitations, including the number of micro-ops that
/// can dispatch per cycle.
///
/// TODO: Also check whether the SU must start a new group.
bool LE1SchedStrategy::SchedBoundary::checkHazard(SUnit *SU) {
  if (HazardRec->isEnabled())
    return HazardRec->getHazardType(SU) != ScheduleHazardRecognizer::NoHazard;

  unsigned uops = SchedModel->getNumMicroOps(SU->getInstr());
  if ((CurrMOps > 0) && (CurrMOps + uops > SchedModel->getIssueWidth())) {
    DEBUG(dbgs() << "  SU(" << SU->NodeNum << ") uops="
          << SchedModel->getNumMicroOps(SU->getInstr()) << '\n');
    return true;
  }
  return false;
}

// Find the unscheduled node in ReadySUs with the highest latency.
unsigned LE1SchedStrategy::SchedBoundary::
findMaxLatency(ArrayRef<SUnit*> ReadySUs) {
  SUnit *LateSU = 0;
  unsigned RemLatency = 0;
  for (ArrayRef<SUnit*>::iterator I = ReadySUs.begin(), E = ReadySUs.end();
       I != E; ++I) {
    unsigned L = getUnscheduledLatency(*I);
    if (L > RemLatency) {
      RemLatency = L;
      LateSU = *I;
    }
  }
  if (LateSU) {
    DEBUG(dbgs() << Available.getName() << " RemLatency SU("
          << LateSU->NodeNum << ") " << RemLatency << "c\n");
  }
  return RemLatency;
}

// Count resources in this zone and the remaining unscheduled
// instruction. Return the max count, scaled. Set OtherCritIdx to the critical
// resource index, or zero if the zone is issue limited.
unsigned LE1SchedStrategy::SchedBoundary::
getOtherResourceCount(unsigned &OtherCritIdx) {
  OtherCritIdx = 0;
  if (!SchedModel->hasInstrSchedModel())
    return 0;

  unsigned OtherCritCount = Rem->RemIssueCount
    + (RetiredMOps * SchedModel->getMicroOpFactor());
  DEBUG(dbgs() << "  " << Available.getName() << " + Remain MOps: "
        << OtherCritCount / SchedModel->getMicroOpFactor() << '\n');
  for (unsigned PIdx = 1, PEnd = SchedModel->getNumProcResourceKinds();
       PIdx != PEnd; ++PIdx) {
    unsigned OtherCount = getResourceCount(PIdx) + Rem->RemainingCounts[PIdx];
    if (OtherCount > OtherCritCount) {
      OtherCritCount = OtherCount;
      OtherCritIdx = PIdx;
    }
  }
  if (OtherCritIdx) {
    DEBUG(dbgs() << "  " << Available.getName() << " + Remain CritRes: "
          << OtherCritCount / SchedModel->getResourceFactor(OtherCritIdx)
          << " " << getResourceName(OtherCritIdx) << "\n");
  }
  return OtherCritCount;
}

/// Set the CandPolicy for this zone given the current resources and latencies
/// inside and outside the zone.
void LE1SchedStrategy::SchedBoundary::setPolicy(CandPolicy &Policy,
                                                   SchedBoundary &OtherZone) {
  // Now that potential stalls have been considered, apply preemptive heuristics
  // based on the the total latency and resources inside and outside this
  // zone.

  // Compute remaining latency. We need this both to determine whether the
  // overall schedule has become latency-limited and whether the instructions
  // outside this zone are resource or latency limited.
  //
  // The "dependent" latency is updated incrementally during scheduling as the
  // max height/depth of scheduled nodes minus the cycles since it was
  // scheduled:
  //   DLat = max (N.depth - (CurrCycle - N.ReadyCycle) for N in Zone
  //
  // The "independent" latency is the max ready queue depth:
  //   ILat = max N.depth for N in Available|Pending
  //
  // RemainingLatency is the greater of independent and dependent latency.
  unsigned RemLatency = DependentLatency;
  RemLatency = std::max(RemLatency, findMaxLatency(Available.elements()));
  RemLatency = std::max(RemLatency, findMaxLatency(Pending.elements()));

  // Compute the critical resource outside the zone.
  unsigned OtherCritIdx;
  unsigned OtherCount = OtherZone.getOtherResourceCount(OtherCritIdx);

  bool OtherResLimited = false;
  if (SchedModel->hasInstrSchedModel()) {
    unsigned LFactor = SchedModel->getLatencyFactor();
    OtherResLimited = (int)(OtherCount - (RemLatency * LFactor)) > (int)LFactor;
  }
  if (!OtherResLimited && (RemLatency + CurrCycle > Rem->CriticalPath)) {
    Policy.ReduceLatency |= true;
    DEBUG(dbgs() << "  " << Available.getName() << " RemainingLatency "
          << RemLatency << " + " << CurrCycle << "c > CritPath "
          << Rem->CriticalPath << "\n");
  }
  // If the same resource is limiting inside and outside the zone, do nothing.
  if (ZoneCritResIdx == OtherCritIdx)
    return;

  DEBUG(
    if (IsResourceLimited) {
      dbgs() << "  " << Available.getName() << " ResourceLimited: "
             << getResourceName(ZoneCritResIdx) << "\n";
    }
    if (OtherResLimited)
      dbgs() << "  RemainingLimit: " << getResourceName(OtherCritIdx) << "\n";
    if (!IsResourceLimited && !OtherResLimited)
      dbgs() << "  Latency limited both directions.\n");

  if (IsResourceLimited && !Policy.ReduceResIdx)
    Policy.ReduceResIdx = ZoneCritResIdx;

  if (OtherResLimited)
    Policy.DemandResIdx = OtherCritIdx;
}

void LE1SchedStrategy::SchedBoundary::releaseNode(SUnit *SU,
                                                     unsigned ReadyCycle) {
  if (ReadyCycle < MinReadyCycle)
    MinReadyCycle = ReadyCycle;

  // Check for interlocks first. For the purpose of other heuristics, an
  // instruction that cannot issue appears as if it's not in the ReadyQueue.
  bool IsBuffered = SchedModel->getMicroOpBufferSize() != 0;
  if ((!IsBuffered && ReadyCycle > CurrCycle) || checkHazard(SU))
    Pending.push(SU);
  else
    Available.push(SU);

  // Record this node as an immediate dependent of the scheduled node.
  NextSUs.insert(SU);
}

/// Move the boundary of scheduled code by one cycle.
void LE1SchedStrategy::SchedBoundary::bumpCycle(unsigned NextCycle) {
  if (SchedModel->getMicroOpBufferSize() == 0) {
    assert(MinReadyCycle < UINT_MAX && "MinReadyCycle uninitialized");
    if (MinReadyCycle > NextCycle)
      NextCycle = MinReadyCycle;
  }
  // Update the current micro-ops, which will issue in the next cycle.
  unsigned DecMOps = SchedModel->getIssueWidth() * (NextCycle - CurrCycle);
  CurrMOps = (CurrMOps <= DecMOps) ? 0 : CurrMOps - DecMOps;

  // Decrement DependentLatency based on the next cycle.
  if ((NextCycle - CurrCycle) > DependentLatency)
    DependentLatency = 0;
  else
    DependentLatency -= (NextCycle - CurrCycle);

  if (!HazardRec->isEnabled()) {
    // Bypass HazardRec virtual calls.
    CurrCycle = NextCycle;
  }
  else {
    // Bypass getHazardType calls in case of long latency.
    for (; CurrCycle != NextCycle; ++CurrCycle) {
      if (isTop())
        HazardRec->AdvanceCycle();
      else
        HazardRec->RecedeCycle();
    }
  }
  CheckPending = true;
  unsigned LFactor = SchedModel->getLatencyFactor();
  IsResourceLimited =
    (int)(getCriticalCount() - (getScheduledLatency() * LFactor))
    > (int)LFactor;

  DEBUG(dbgs() << "Cycle: " << CurrCycle << ' ' << Available.getName() << '\n');
}

void LE1SchedStrategy::SchedBoundary::incExecutedResources(unsigned PIdx,
                                                              unsigned Count) {
  ExecutedResCounts[PIdx] += Count;
  if (ExecutedResCounts[PIdx] > MaxExecutedResCount)
    MaxExecutedResCount = ExecutedResCounts[PIdx];
}

/// Add the given processor resource to this scheduled zone.
///
/// \param Cycles indicates the number of consecutive (non-pipelined) cycles
/// during which this resource is consumed.
///
/// \return the next cycle at which the instruction may execute without
/// oversubscribing resources.
unsigned LE1SchedStrategy::SchedBoundary::
countResource(unsigned PIdx, unsigned Cycles, unsigned ReadyCycle) {
  unsigned Factor = SchedModel->getResourceFactor(PIdx);
  unsigned Count = Factor * Cycles;
  DEBUG(dbgs() << "  " << getResourceName(PIdx)
        << " +" << Cycles << "x" << Factor << "u\n");

  // Update Executed resources counts.
  incExecutedResources(PIdx, Count);
  assert(Rem->RemainingCounts[PIdx] >= Count && "resource double counted");
  Rem->RemainingCounts[PIdx] -= Count;

  // Check if this resource exceeds the current critical resource. If so, it
  // becomes the critical resource.
  if (ZoneCritResIdx != PIdx && (getResourceCount(PIdx) > getCriticalCount())) {
    ZoneCritResIdx = PIdx;
    DEBUG(dbgs() << "  *** Critical resource "
          << getResourceName(PIdx) << ": "
          << getResourceCount(PIdx) / SchedModel->getLatencyFactor() << "c\n");
  }
  // TODO: We don't yet model reserved resources. It's not hard though.
  return CurrCycle;
}

/// Move the boundary of scheduled code by one SUnit.
void LE1SchedStrategy::SchedBoundary::bumpNode(SUnit *SU) {
  // Update the reservation table.
  if (HazardRec->isEnabled()) {
    if (!isTop() && SU->isCall) {
      // Calls are scheduled with their preceding instructions. For bottom-up
      // scheduling, clear the pipeline state before emitting.
      HazardRec->Reset();
    }
    HazardRec->EmitInstruction(SU);
  }
  const MCSchedClassDesc *SC = DAG->getSchedClass(SU);
  unsigned IncMOps = SchedModel->getNumMicroOps(SU->getInstr());
  CurrMOps += IncMOps;
  // checkHazard prevents scheduling multiple instructions per cycle that exceed
  // issue width. However, we commonly reach the maximum. In this case
  // opportunistically bump the cycle to avoid uselessly checking everything in
  // the readyQ. Furthermore, a single instruction may produce more than one
  // cycle's worth of micro-ops.
  //
  // TODO: Also check if this SU must end a dispatch group.
  unsigned NextCycle = CurrCycle;
  if (CurrMOps >= SchedModel->getIssueWidth()) {
    ++NextCycle;
    DEBUG(dbgs() << "  *** Max MOps " << CurrMOps
          << " at cycle " << CurrCycle << '\n');
  }
  unsigned ReadyCycle = (isTop() ? SU->TopReadyCycle : SU->BotReadyCycle);
  DEBUG(dbgs() << "  Ready @" << ReadyCycle << "c\n");

  switch (SchedModel->getMicroOpBufferSize()) {
  case 0:
    assert(ReadyCycle <= CurrCycle && "Broken PendingQueue");
    break;
  case 1:
    if (ReadyCycle > NextCycle) {
      NextCycle = ReadyCycle;
      DEBUG(dbgs() << "  *** Stall until: " << ReadyCycle << "\n");
    }
    break;
  default:
    // We don't currently model the OOO reorder buffer, so consider all
    // scheduled MOps to be "retired".
    break;
  }
  RetiredMOps += IncMOps;

  // Update resource counts and critical resource.
  if (SchedModel->hasInstrSchedModel()) {
    unsigned DecRemIssue = IncMOps * SchedModel->getMicroOpFactor();
    assert(Rem->RemIssueCount >= DecRemIssue && "MOps double counted");
    Rem->RemIssueCount -= DecRemIssue;
    if (ZoneCritResIdx) {
      // Scale scheduled micro-ops for comparing with the critical resource.
      unsigned ScaledMOps =
        RetiredMOps * SchedModel->getMicroOpFactor();

      // If scaled micro-ops are now more than the previous critical resource by
      // a full cycle, then micro-ops issue becomes critical.
      if ((int)(ScaledMOps - getResourceCount(ZoneCritResIdx))
          >= (int)SchedModel->getLatencyFactor()) {
        ZoneCritResIdx = 0;
        DEBUG(dbgs() << "  *** Critical resource NumMicroOps: "
              << ScaledMOps / SchedModel->getLatencyFactor() << "c\n");
      }
    }
    for (TargetSchedModel::ProcResIter
           PI = SchedModel->getWriteProcResBegin(SC),
           PE = SchedModel->getWriteProcResEnd(SC); PI != PE; ++PI) {
      unsigned RCycle =
        countResource(PI->ProcResourceIdx, PI->Cycles, ReadyCycle);
      if (RCycle > NextCycle)
        NextCycle = RCycle;
    }
  }
  // Update ExpectedLatency and DependentLatency.
  unsigned &TopLatency = isTop() ? ExpectedLatency : DependentLatency;
  unsigned &BotLatency = isTop() ? DependentLatency : ExpectedLatency;
  if (SU->getDepth() > TopLatency) {
    TopLatency = SU->getDepth();
    DEBUG(dbgs() << "  " << Available.getName()
          << " TopLatency SU(" << SU->NodeNum << ") " << TopLatency << "c\n");
  }
  if (SU->getHeight() > BotLatency) {
    BotLatency = SU->getHeight();
    DEBUG(dbgs() << "  " << Available.getName()
          << " BotLatency SU(" << SU->NodeNum << ") " << BotLatency << "c\n");
  }
  // If we stall for any reason, bump the cycle.
  if (NextCycle > CurrCycle) {
    bumpCycle(NextCycle);
  }
  else {
    // After updating ZoneCritResIdx and ExpectedLatency, check if we're
    // resource limited. If a stall occured, bumpCycle does this.
    unsigned LFactor = SchedModel->getLatencyFactor();
    IsResourceLimited =
      (int)(getCriticalCount() - (getScheduledLatency() * LFactor))
      > (int)LFactor;
  }
  DEBUG(dumpScheduledState());
}

/// Release pending ready nodes in to the available queue. This makes them
/// visible to heuristics.
void LE1SchedStrategy::SchedBoundary::releasePending() {
  // If the available queue is empty, it is safe to reset MinReadyCycle.
  if (Available.empty())
    MinReadyCycle = UINT_MAX;

  // Check to see if any of the pending instructions are ready to issue.  If
  // so, add them to the available queue.
  bool IsBuffered = SchedModel->getMicroOpBufferSize() != 0;
  for (unsigned i = 0, e = Pending.size(); i != e; ++i) {
    SUnit *SU = *(Pending.begin()+i);
    unsigned ReadyCycle = isTop() ? SU->TopReadyCycle : SU->BotReadyCycle;

    if (ReadyCycle < MinReadyCycle)
      MinReadyCycle = ReadyCycle;

    if (!IsBuffered && ReadyCycle > CurrCycle)
      continue;

    if (checkHazard(SU))
      continue;

    Available.push(SU);
    Pending.remove(Pending.begin()+i);
    --i; --e;
  }
  DEBUG(if (!Pending.empty()) Pending.dump());
  CheckPending = false;
}

/// Remove SU from the ready set for this boundary.
void LE1SchedStrategy::SchedBoundary::removeReady(SUnit *SU) {
  if (Available.isInQueue(SU))
    Available.remove(Available.find(SU));
  else {
    assert(Pending.isInQueue(SU) && "bad ready count");
    Pending.remove(Pending.find(SU));
  }
}

/// If this queue only has one ready candidate, return it. As a side effect,
/// defer any nodes that now hit a hazard, and advance the cycle until at least
/// one node is ready. If multiple instructions are ready, return NULL.
SUnit *LE1SchedStrategy::SchedBoundary::pickOnlyChoice() {
  if (CheckPending)
    releasePending();

  if (CurrMOps > 0) {
    // Defer any ready instrs that now have a hazard.
    for (ReadyQueue::iterator I = Available.begin(); I != Available.end();) {
      if (checkHazard(*I)) {
        Pending.push(*I);
        I = Available.remove(I);
        continue;
      }
      ++I;
    }
  }
  for (unsigned i = 0; Available.empty(); ++i) {
    assert(i <= (HazardRec->getMaxLookAhead() + MaxObservedLatency) &&
           "permanent hazard"); (void)i;
    bumpCycle(CurrCycle + 1);
    releasePending();
  }
  if (Available.size() == 1)
    return *Available.begin();
  return NULL;
}

#ifndef NDEBUG
// This is useful information to dump after bumpNode.
// Note that the Queue contents are more useful before pickNodeFromQueue.
void LE1SchedStrategy::SchedBoundary::dumpScheduledState() {
  unsigned ResFactor;
  unsigned ResCount;
  if (ZoneCritResIdx) {
    ResFactor = SchedModel->getResourceFactor(ZoneCritResIdx);
    ResCount = getResourceCount(ZoneCritResIdx);
  }
  else {
    ResFactor = SchedModel->getMicroOpFactor();
    ResCount = RetiredMOps * SchedModel->getMicroOpFactor();
  }
  unsigned LFactor = SchedModel->getLatencyFactor();
  dbgs() << Available.getName() << " @" << CurrCycle << "c\n"
         << "  Retired: " << RetiredMOps;
  dbgs() << "\n  Executed: " << getExecutedCount() / LFactor << "c";
  dbgs() << "\n  Critical: " << ResCount / LFactor << "c, "
         << ResCount / ResFactor << " " << getResourceName(ZoneCritResIdx)
         << "\n  ExpectedLatency: " << ExpectedLatency << "c\n"
         << (IsResourceLimited ? "  - Resource" : "  - Latency")
         << " limited.\n";
}
#endif

void LE1SchedStrategy::SchedCandidate::
initResourceDelta(const ScheduleDAGMI *DAG,
                  const TargetSchedModel *SchedModel) {
  if (!Policy.ReduceResIdx && !Policy.DemandResIdx)
    return;

  const MCSchedClassDesc *SC = DAG->getSchedClass(SU);
  for (TargetSchedModel::ProcResIter
         PI = SchedModel->getWriteProcResBegin(SC),
         PE = SchedModel->getWriteProcResEnd(SC); PI != PE; ++PI) {
    if (PI->ProcResourceIdx == Policy.ReduceResIdx)
      ResDelta.CritResources += PI->Cycles;
    if (PI->ProcResourceIdx == Policy.DemandResIdx)
      ResDelta.DemandedResources += PI->Cycles;
  }
}


/// Return true if this heuristic determines order.
static bool tryLess(int TryVal, int CandVal,
                    LE1SchedStrategy::SchedCandidate &TryCand,
                    LE1SchedStrategy::SchedCandidate &Cand,
                    LE1SchedStrategy::CandReason Reason) {
  if (TryVal < CandVal) {
    TryCand.Reason = Reason;
    return true;
  }
  if (TryVal > CandVal) {
    if (Cand.Reason > Reason)
      Cand.Reason = Reason;
    return true;
  }
  Cand.setRepeat(Reason);
  return false;
}

static bool tryGreater(int TryVal, int CandVal,
                       LE1SchedStrategy::SchedCandidate &TryCand,
                       LE1SchedStrategy::SchedCandidate &Cand,
                       LE1SchedStrategy::CandReason Reason) {
  if (TryVal > CandVal) {
    TryCand.Reason = Reason;
    return true;
  }
  if (TryVal < CandVal) {
    if (Cand.Reason > Reason)
      Cand.Reason = Reason;
    return true;
  }
  Cand.setRepeat(Reason);
  return false;
}

static bool tryPressure(const PressureChange &TryP,
                        const PressureChange &CandP,
                        LE1SchedStrategy::SchedCandidate &TryCand,
                        LE1SchedStrategy::SchedCandidate &Cand,
                        LE1SchedStrategy::CandReason Reason) {
  int TryRank = TryP.getPSetOrMax();
  int CandRank = CandP.getPSetOrMax();
  // If both candidates affect the same set, go with the smallest increase.
  if (TryRank == CandRank) {
    return tryLess(TryP.getUnitInc(), CandP.getUnitInc(), TryCand, Cand,
                   Reason);
  }
  // If one candidate decreases and the other increases, go with it.
  // Invalid candidates have UnitInc==0.
  if (tryLess(TryP.getUnitInc() < 0, CandP.getUnitInc() < 0, TryCand, Cand,
              Reason)) {
    return true;
  }
  // If the candidates are decreasing pressure, reverse priority.
  if (TryP.getUnitInc() < 0)
    std::swap(TryRank, CandRank);
  return tryGreater(TryRank, CandRank, TryCand, Cand, Reason);
}

static unsigned getWeakLeft(const SUnit *SU, bool isTop) {
  return (isTop) ? SU->WeakPredsLeft : SU->WeakSuccsLeft;
}

/// Minimize physical register live ranges. Regalloc wants them adjacent to
/// their physreg def/use.
///
/// FIXME: This is an unnecessary check on the critical path. Most are root/leaf
/// copies which can be prescheduled. The rest (e.g. x86 MUL) could be bundled
/// with the operation that produces or consumes the physreg. We'll do this when
/// regalloc has support for parallel copies.
static int biasPhysRegCopy(const SUnit *SU, bool isTop) {
  const MachineInstr *MI = SU->getInstr();
  if (!MI->isCopy())
    return 0;

  unsigned ScheduledOper = isTop ? 1 : 0;
  unsigned UnscheduledOper = isTop ? 0 : 1;
  // If we have already scheduled the physreg produce/consumer, immediately
  // schedule the copy.
  if (TargetRegisterInfo::isPhysicalRegister(
        MI->getOperand(ScheduledOper).getReg()))
    return 1;
  // If the physreg is at the boundary, defer it. Otherwise schedule it
  // immediately to free the dependent. We can hoist the copy later.
  bool AtBoundary = isTop ? !SU->NumSuccsLeft : !SU->NumPredsLeft;
  if (TargetRegisterInfo::isPhysicalRegister(
        MI->getOperand(UnscheduledOper).getReg()))
    return AtBoundary ? -1 : 1;
  return 0;
}

static bool tryLatency(LE1SchedStrategy::SchedCandidate &TryCand,
                       LE1SchedStrategy::SchedCandidate &Cand,
                       LE1SchedStrategy::SchedBoundary &Zone) {
  if (Zone.isTop()) {
    if (Cand.SU->getDepth() > Zone.getScheduledLatency()) {
      if (tryLess(TryCand.SU->getDepth(), Cand.SU->getDepth(),
                  TryCand, Cand, LE1SchedStrategy::TopDepthReduce))
        return true;
    }
    if (tryGreater(TryCand.SU->getHeight(), Cand.SU->getHeight(),
                   TryCand, Cand, LE1SchedStrategy::TopPathReduce))
      return true;
  }
  else {
    if (Cand.SU->getHeight() > Zone.getScheduledLatency()) {
      if (tryLess(TryCand.SU->getHeight(), Cand.SU->getHeight(),
                  TryCand, Cand, LE1SchedStrategy::BotHeightReduce))
        return true;
    }
    if (tryGreater(TryCand.SU->getDepth(), Cand.SU->getDepth(),
                   TryCand, Cand, LE1SchedStrategy::BotPathReduce))
      return true;
  }
  return false;
}

/// Apply a set of heursitics to a new candidate. Heuristics are currently
/// hierarchical. This may be more efficient than a graduated cost model because
/// we don't need to evaluate all aspects of the model for each node in the
/// queue. But it's really done to make the heuristics easier to debug and
/// statistically analyze.
///
/// \param Cand provides the policy and current best candidate.
/// \param TryCand refers to the next SUnit candidate, otherwise uninitialized.
/// \param Zone describes the scheduled zone that we are extending.
/// \param RPTracker describes reg pressure within the scheduled zone.
/// \param TempTracker is a scratch pressure tracker to reuse in queries.
void LE1SchedStrategy::tryCandidate(SchedCandidate &Cand,
                                       SchedCandidate &TryCand,
                                       SchedBoundary &Zone,
                                       const RegPressureTracker &RPTracker,
                                       RegPressureTracker &TempTracker) {

  if (DAG->isTrackingPressure()) {
    // Always initialize TryCand's RPDelta.
    if (Zone.isTop()) {
      TempTracker.getMaxDownwardPressureDelta(
        TryCand.SU->getInstr(),
        TryCand.RPDelta,
        DAG->getRegionCriticalPSets(),
        DAG->getRegPressure().MaxSetPressure);
    }
    else {
      /*
      if (VerifyScheduling) {
        TempTracker.getMaxUpwardPressureDelta(
          TryCand.SU->getInstr(),
          &DAG->getPressureDiff(TryCand.SU),
          TryCand.RPDelta,
          DAG->getRegionCriticalPSets(),
          DAG->getRegPressure().MaxSetPressure);
      }
      else {*/
        RPTracker.getUpwardPressureDelta(
          TryCand.SU->getInstr(),
          DAG->getPressureDiff(TryCand.SU),
          TryCand.RPDelta,
          DAG->getRegionCriticalPSets(),
          DAG->getRegPressure().MaxSetPressure);
      }
    }
  DEBUG(if (TryCand.RPDelta.Excess.isValid())
          dbgs() << "  SU(" << TryCand.SU->NodeNum << ") "
                 << TRI->getRegPressureSetName(TryCand.RPDelta.Excess.getPSet())
                 << ":" << TryCand.RPDelta.Excess.getUnitInc() << "\n");

  // Initialize the candidate if needed.
  if (!Cand.isValid()) {
    TryCand.Reason = NodeOrder;
    return;
  }

  if (tryGreater(biasPhysRegCopy(TryCand.SU, Zone.isTop()),
                 biasPhysRegCopy(Cand.SU, Zone.isTop()),
                 TryCand, Cand, PhysRegCopy))
    return;

  // Avoid exceeding the target's limit. If signed PSetID is negative, it is
  // invalid; convert it to INT_MAX to give it lowest priority.
  if (DAG->isTrackingPressure() && tryPressure(TryCand.RPDelta.Excess,
                                               Cand.RPDelta.Excess,
                                               TryCand, Cand, RegExcess))
    return;

  // Avoid increasing the max critical pressure in the scheduled region.
  if (DAG->isTrackingPressure() && tryPressure(TryCand.RPDelta.CriticalMax,
                                               Cand.RPDelta.CriticalMax,
                                               TryCand, Cand, RegCritical))
    return;

  // For loops that are acyclic path limited, aggressively schedule for latency.
  // This can result in very long dependence chains scheduled in sequence, so
  // once every cycle (when CurrMOps == 0), switch to normal heuristics.
  if (Rem.IsAcyclicLatencyLimited && !Zone.CurrMOps
      && tryLatency(TryCand, Cand, Zone))
    return;

  // Keep clustered nodes together to encourage downstream peephole
  // optimizations which may reduce resource requirements.
  //
  // This is a best effort to set things up for a post-RA pass. Optimizations
  // like generating loads of multiple registers should ideally be done within
  // the scheduler pass by combining the loads during DAG postprocessing.
  const SUnit *NextClusterSU =
    Zone.isTop() ? DAG->getNextClusterSucc() : DAG->getNextClusterPred();
  if (tryGreater(TryCand.SU == NextClusterSU, Cand.SU == NextClusterSU,
                 TryCand, Cand, Cluster))
    return;

  // Weak edges are for clustering and other constraints.
  if (tryLess(getWeakLeft(TryCand.SU, Zone.isTop()),
              getWeakLeft(Cand.SU, Zone.isTop()),
              TryCand, Cand, Weak)) {
    return;
  }
  // Avoid increasing the max pressure of the entire region.
  if (DAG->isTrackingPressure() && tryPressure(TryCand.RPDelta.CurrentMax,
                                               Cand.RPDelta.CurrentMax,
                                               TryCand, Cand, RegMax))
    return;

  // Avoid critical resource consumption and balance the schedule.
  TryCand.initResourceDelta(DAG, SchedModel);
  if (tryLess(TryCand.ResDelta.CritResources, Cand.ResDelta.CritResources,
              TryCand, Cand, ResourceReduce))
    return;
  if (tryGreater(TryCand.ResDelta.DemandedResources,
                 Cand.ResDelta.DemandedResources,
                 TryCand, Cand, ResourceDemand))
    return;

  // Avoid serializing long latency dependence chains.
  // For acyclic path limited loops, latency was already checked above.
  if (Cand.Policy.ReduceLatency && !Rem.IsAcyclicLatencyLimited
      && tryLatency(TryCand, Cand, Zone)) {
    return;
  }

  // Prefer immediate defs/users of the last scheduled instruction. This is a
  // local pressure avoidance strategy that also makes the machine code
  // readable.
  if (tryGreater(Zone.NextSUs.count(TryCand.SU), Zone.NextSUs.count(Cand.SU),
                 TryCand, Cand, NextDefUse))
    return;

  // Fall through to original instruction order.
  if ((Zone.isTop() && TryCand.SU->NodeNum < Cand.SU->NodeNum)
      || (!Zone.isTop() && TryCand.SU->NodeNum > Cand.SU->NodeNum)) {
    TryCand.Reason = NodeOrder;
  }
}

#ifndef NDEBUG
const char *LE1SchedStrategy::getReasonStr(
  LE1SchedStrategy::CandReason Reason) {
  switch (Reason) {
  case NoCand:         return "NOCAND    ";
  case PhysRegCopy:    return "PREG-COPY";
  case RegExcess:      return "REG-EXCESS";
  case RegCritical:    return "REG-CRIT  ";
  case Cluster:        return "CLUSTER   ";
  case Weak:           return "WEAK      ";
  case RegMax:         return "REG-MAX   ";
  case ResourceReduce: return "RES-REDUCE";
  case ResourceDemand: return "RES-DEMAND";
  case TopDepthReduce: return "TOP-DEPTH ";
  case TopPathReduce:  return "TOP-PATH  ";
  case BotHeightReduce:return "BOT-HEIGHT";
  case BotPathReduce:  return "BOT-PATH  ";
  case NextDefUse:     return "DEF-USE   ";
  case NodeOrder:      return "ORDER     ";
  };
  llvm_unreachable("Unknown reason!");
}

void LE1SchedStrategy::traceCandidate(const SchedCandidate &Cand) {
  PressureChange P;
  unsigned ResIdx = 0;
  unsigned Latency = 0;
  switch (Cand.Reason) {
  default:
    break;
  case RegExcess:
    P = Cand.RPDelta.Excess;
    break;
  case RegCritical:
    P = Cand.RPDelta.CriticalMax;
    break;
  case RegMax:
    P = Cand.RPDelta.CurrentMax;
    break;
  case ResourceReduce:
    ResIdx = Cand.Policy.ReduceResIdx;
    break;
  case ResourceDemand:
    ResIdx = Cand.Policy.DemandResIdx;
    break;
  case TopDepthReduce:
    Latency = Cand.SU->getDepth();
    break;
  case TopPathReduce:
    Latency = Cand.SU->getHeight();
    break;
  case BotHeightReduce:
    Latency = Cand.SU->getHeight();
    break;
  case BotPathReduce:
    Latency = Cand.SU->getDepth();
    break;
  }
  dbgs() << "  SU(" << Cand.SU->NodeNum << ") " << getReasonStr(Cand.Reason);
  if (P.isValid())
    dbgs() << " " << TRI->getRegPressureSetName(P.getPSet())
           << ":" << P.getUnitInc() << " ";
  else
    dbgs() << "      ";
  if (ResIdx)
    dbgs() << " " << SchedModel->getProcResource(ResIdx)->Name << " ";
  else
    dbgs() << "         ";
  if (Latency)
    dbgs() << " " << Latency << " cycles ";
  else
    dbgs() << "          ";
  dbgs() << '\n';
}
#endif

/// Pick the best candidate from the queue.
///
/// TODO: getMaxPressureDelta results can be mostly cached for each SUnit during
/// DAG building. To adjust for the current scheduling location we need to
/// maintain the number of vreg uses remaining to be top-scheduled.
void LE1SchedStrategy::pickNodeFromQueue(SchedBoundary &Zone,
                                            const RegPressureTracker &RPTracker,
                                            SchedCandidate &Cand) {
  ReadyQueue &Q = Zone.Available;

  DEBUG(Q.dump());

  // getMaxPressureDelta temporarily modifies the tracker.
  RegPressureTracker &TempTracker = const_cast<RegPressureTracker&>(RPTracker);

  for (ReadyQueue::iterator I = Q.begin(), E = Q.end(); I != E; ++I) {

    SchedCandidate TryCand(Cand.Policy);
    TryCand.SU = *I;
    tryCandidate(Cand, TryCand, Zone, RPTracker, TempTracker);
    if (TryCand.Reason != NoCand) {
      // Initialize resource delta if needed in case future heuristics query it.
      if (TryCand.ResDelta == SchedResourceDelta())
        TryCand.initResourceDelta(DAG, SchedModel);
      Cand.setBest(TryCand);
      DEBUG(traceCandidate(Cand));
    }
  }
}

static void tracePick(const LE1SchedStrategy::SchedCandidate &Cand,
                      bool IsTop) {
  DEBUG(dbgs() << "Pick " << (IsTop ? "Top " : "Bot ")
        << LE1SchedStrategy::getReasonStr(Cand.Reason) << '\n');
}

/// Pick the best candidate node from either the top or bottom queue.
SUnit *LE1SchedStrategy::pickNodeBidirectional(bool &IsTopNode) {
  // Schedule as far as possible in the direction of no choice. This is most
  // efficient, but also provides the best heuristics for CriticalPSets.
  if (SUnit *SU = Bot.pickOnlyChoice()) {
    IsTopNode = false;
    DEBUG(dbgs() << "Pick Bot NOCAND\n");
    return SU;
  }
  if (SUnit *SU = Top.pickOnlyChoice()) {
    IsTopNode = true;
    DEBUG(dbgs() << "Pick Top NOCAND\n");
    return SU;
  }
  CandPolicy NoPolicy;
  SchedCandidate BotCand(NoPolicy);
  SchedCandidate TopCand(NoPolicy);
  Bot.setPolicy(BotCand.Policy, Top);
  Top.setPolicy(TopCand.Policy, Bot);

  // Prefer bottom scheduling when heuristics are silent.
  pickNodeFromQueue(Bot, DAG->getBotRPTracker(), BotCand);
  assert(BotCand.Reason != NoCand && "failed to find the first candidate");

  // If either Q has a single candidate that provides the least increase in
  // Excess pressure, we can immediately schedule from that Q.
  //
  // RegionCriticalPSets summarizes the pressure within the scheduled region and
  // affects picking from either Q. If scheduling in one direction must
  // increase pressure for one of the excess PSets, then schedule in that
  // direction first to provide more freedom in the other direction.
  if ((BotCand.Reason == RegExcess && !BotCand.isRepeat(RegExcess))
      || (BotCand.Reason == RegCritical
          && !BotCand.isRepeat(RegCritical)))
  {
    IsTopNode = false;
    tracePick(BotCand, IsTopNode);
    return BotCand.SU;
  }
  // Check if the top Q has a better candidate.
  pickNodeFromQueue(Top, DAG->getTopRPTracker(), TopCand);
  assert(TopCand.Reason != NoCand && "failed to find the first candidate");

  // Choose the queue with the most important (lowest enum) reason.
  if (TopCand.Reason < BotCand.Reason) {
    IsTopNode = true;
    tracePick(TopCand, IsTopNode);
    return TopCand.SU;
  }
  // Otherwise prefer the bottom candidate, in node order if all else failed.
  IsTopNode = false;
  tracePick(BotCand, IsTopNode);
  return BotCand.SU;
}

/// Pick the best node to balance the schedule. Implements MachineSchedStrategy.
SUnit *LE1SchedStrategy::pickNode(bool &IsTopNode) {
  if (DAG->top() == DAG->bottom()) {
    assert(Top.Available.empty() && Top.Pending.empty() &&
           Bot.Available.empty() && Bot.Pending.empty() && "ReadyQ garbage");
    return NULL;
  }
  SUnit *SU;
  do {
    if (RegionPolicy.OnlyTopDown) {
      SU = Top.pickOnlyChoice();
      if (!SU) {
        CandPolicy NoPolicy;
        SchedCandidate TopCand(NoPolicy);
        pickNodeFromQueue(Top, DAG->getTopRPTracker(), TopCand);
        assert(TopCand.Reason != NoCand && "failed to find a candidate");
        tracePick(TopCand, true);
        SU = TopCand.SU;
      }
      IsTopNode = true;
    }
    else if (RegionPolicy.OnlyBottomUp) {
      SU = Bot.pickOnlyChoice();
      if (!SU) {
        CandPolicy NoPolicy;
        SchedCandidate BotCand(NoPolicy);
        pickNodeFromQueue(Bot, DAG->getBotRPTracker(), BotCand);
        assert(BotCand.Reason != NoCand && "failed to find a candidate");
        tracePick(BotCand, false);
        SU = BotCand.SU;
      }
      IsTopNode = false;
    }
    else {
      SU = pickNodeBidirectional(IsTopNode);
    }
  } while (SU->isScheduled);

  if (SU->isTopReady())
    Top.removeReady(SU);
  if (SU->isBottomReady())
    Bot.removeReady(SU);

  DEBUG(dbgs() << "Scheduling SU(" << SU->NodeNum << ") " << *SU->getInstr());
  return SU;
}

void LE1SchedStrategy::reschedulePhysRegCopies(SUnit *SU, bool isTop) {

  MachineBasicBlock::iterator InsertPos = SU->getInstr();
  if (!isTop)
    ++InsertPos;
  SmallVectorImpl<SDep> &Deps = isTop ? SU->Preds : SU->Succs;

  // Find already scheduled copies with a single physreg dependence and move
  // them just above the scheduled instruction.
  for (SmallVectorImpl<SDep>::iterator I = Deps.begin(), E = Deps.end();
       I != E; ++I) {
    if (I->getKind() != SDep::Data || !TRI->isPhysicalRegister(I->getReg()))
      continue;
    SUnit *DepSU = I->getSUnit();
    if (isTop ? DepSU->Succs.size() > 1 : DepSU->Preds.size() > 1)
      continue;
    MachineInstr *Copy = DepSU->getInstr();
    if (!Copy->isCopy())
      continue;
    DEBUG(dbgs() << "  Rescheduling physreg copy ";
          I->getSUnit()->dump(DAG));
    DAG->moveInstruction(Copy, InsertPos);
  }
}

/// Update the scheduler's state after scheduling a node. This is the same node
/// that was just returned by pickNode(). However, ScheduleDAGMI needs to update
/// it's state based on the current cycle before MachineSchedStrategy does.
///
/// FIXME: Eventually, we may bundle physreg copies rather than rescheduling
/// them here. See comments in biasPhysRegCopy.
void LE1SchedStrategy::schedNode(SUnit *SU, bool IsTopNode) {
  if (IsTopNode) {
    SU->TopReadyCycle = std::max(SU->TopReadyCycle, Top.CurrCycle);
    Top.bumpNode(SU);
    if (SU->hasPhysRegUses)
      reschedulePhysRegCopies(SU, true);
  }
  else {
    SU->BotReadyCycle = std::max(SU->BotReadyCycle, Bot.CurrCycle);
    Bot.bumpNode(SU);
    if (SU->hasPhysRegDefs)
      reschedulePhysRegCopies(SU, false);
  }
}
