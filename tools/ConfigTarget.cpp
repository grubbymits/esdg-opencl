// BLAH
// MachineBlockFrequencyInfo - Machine block frequency analysis, use this in
// conjunction with the size of the block to create a weighting for the block.
// Use blocks that account for 60% of the execution or the top 5.
// Need to consider pipeline latencies?
// Assume unlimited functional units.
// Have a schedule list, each ‘cycle’ note the FU usage of the ready
// instructions and then schedule them. At the end of the block we’d have X
// cycles of functional unit usage and we could take an average, with there
// being at least one of each FU.

/// Top-level MachineScheduler pass driver.
///
/// Visit blocks in function order. Divide each block into scheduling regions
/// and visit them bottom-up. Visiting regions bottom-up is not required, but is
/// consistent with the DAG builder, which traverses the interior of the
/// scheduling regions bottom-up.
///
/// This design avoids exposing scheduling boundaries to the DAG builder,
/// simplifying the DAG builder's support for "special" target instructions.
/// At the same time the design allows target schedulers to operate across
/// scheduling boundaries, for example to bundle the boudary instructions
/// without reordering them. This creates complexity, because the target
/// scheduler must update the RegionBegin and RegionEnd positions cached by
/// ScheduleDAGInstrs whenever adding or removing instructions. A much simpler
/// design would be to split blocks at scheduling boundaries, but LLVM has a
/// general bias against block splitting purely for implementation simplicity.
bool MachineScheduler::runOnMachineFunction(MachineFunction &mf) {
  DEBUG(dbgs() << "Before MISsched:\n"; mf.print(dbgs()));

  // Initialize the context of the pass.
  MF = &mf;
  MLI = &getAnalysis<MachineLoopInfo>();
  MDT = &getAnalysis<MachineDominatorTree>();
  PassConfig = &getAnalysis<TargetPassConfig>();
  AA = &getAnalysis<AliasAnalysis>();

  LIS = &getAnalysis<LiveIntervals>();

  // Also get BlockFrequencyInfo

  // Not sure if we'll be able to have this? Or maybe when we use this pass, we
  // just have the default scalar target?
  const TargetInstrInfo *TII = MF->getTarget().getInstrInfo();

  if (VerifyScheduling) {
    DEBUG(LIS->dump());
    MF->verify(this, "Before machine scheduling.");
  }
  RegClassInfo->runOnMachineFunction(*MF);

  // Instantiate the selected scheduler for this target, function, and
  // optimization level.
  OwningPtr<ScheduleDAGInstrs> Scheduler(createMachineScheduler());

  // Use profile info to get the blocks which account for 50% of the function.
  // Don't visit all blocks, just the ones we've found to be executed the most.
  //
  // TODO: Visit blocks in global postorder or postorder within the bottom-up
  // loop tree. Then we can optionally compute global RegPressure.
  for (MachineFunction::iterator MBB = MF->begin(), MBBEnd = MF->end();
       MBB != MBBEnd; ++MBB) {

    Scheduler->startBlock(MBB);

    // Break the block into scheduling regions [I, RegionEnd), and schedule each
    // region as soon as it is discovered. RegionEnd points the scheduling
    // boundary at the bottom of the region. The DAG does not include RegionEnd,
    // but the region does (i.e. the next RegionEnd is above the previous
    // RegionBegin). If the current block has no terminator then RegionEnd ==
    // MBB->end() for the bottom region.
    //
    // The Scheduler may insert instructions during either schedule() or
    // exitRegion(), even for empty regions. So the local iterators 'I' and
    // 'RegionEnd' are invalid across these calls.
    unsigned RemainingInstrs = MBB->size();
    for(MachineBasicBlock::iterator RegionEnd = MBB->end();
        RegionEnd != MBB->begin(); RegionEnd = Scheduler->begin()) {

      // Avoid decrementing RegionEnd for blocks with no terminator.
      if (RegionEnd != MBB->end()
          || TII->isSchedulingBoundary(llvm::prior(RegionEnd), MBB, *MF)) {
        --RegionEnd;
        // Count the boundary instruction.
        --RemainingInstrs;
      }

      // The next region starts above the previous region. Look backward in the
      // instruction stream until we find the nearest boundary.
      unsigned NumRegionInstrs = 0;
      MachineBasicBlock::iterator I = RegionEnd;
      for(;I != MBB->begin(); --I, --RemainingInstrs, ++NumRegionInstrs) {
        if (TII->isSchedulingBoundary(llvm::prior(I), MBB, *MF))
          break;
      }
      // Notify the scheduler of the region, even if we may skip scheduling
      // it. Perhaps it still needs to be bundled.
      Scheduler->enterRegion(MBB, I, RegionEnd, NumRegionInstrs);

      // Skip empty scheduling regions (0 or 1 schedulable instructions).
      if (I == RegionEnd || I == llvm::prior(RegionEnd)) {
        // Close the current region. Bundle the terminator if needed.
        // This invalidates 'RegionEnd' and 'I'.
        Scheduler->exitRegion();
        continue;
      }
      DEBUG(dbgs() << "********** MI Scheduling **********\n");
      DEBUG(dbgs() << MF->getName()
            << ":BB#" << MBB->getNumber() << " " << MBB->getName()
            << "\n  From: " << *I << "    To: ";
            if (RegionEnd != MBB->end()) dbgs() << *RegionEnd;
            else dbgs() << "End";
            dbgs() << " RegionInstrs: " << NumRegionInstrs
            << " Remaining: " << RemainingInstrs << "\n");

      // Schedule a region: possibly reorder instructions.
      // This invalidates 'RegionEnd' and 'I'.
      Scheduler->schedule();

      // Close the current region.
      Scheduler->exitRegion();

      // Scheduling has invalidated the current iterator 'I'. Ask the
      // scheduler for the top of it's scheduled region.
      RegionEnd = Scheduler->begin();
    }
    assert(RemainingInstrs == 0 && "Instruction count mismatch!");
    Scheduler->finishBlock();
  }
  Scheduler->finalizeSchedule();
  DEBUG(LIS->dump());
  if (VerifyScheduling)
    MF->verify(this, "After machine scheduling.");
  return true;
}
/// schedule - Called back from MachineScheduler::runOnMachineFunction
/// after setting up the current scheduling region. [RegionBegin, RegionEnd)
/// only includes instructions that have DAG nodes, not scheduling boundaries.
///
/// This is a skeletal driver, with all the functionality pushed into helpers,
/// so that it can be easilly extended by experimental schedulers. Generally,
/// implementing MachineSchedStrategy should be sufficient to implement a new
/// scheduling algorithm. However, if a scheduler further subclasses
/// ScheduleDAGMI then it will want to override this virtual method in order to
/// update any specialized state.
void ScheduleDAGMI::schedule() {
  buildDAGWithRegPressure();

  Topo.InitDAGTopologicalSorting();

  postprocessDAG();

  SmallVector<SUnit*, 8> TopRoots, BotRoots;
  findRootsAndBiasEdges(TopRoots, BotRoots);

  // Initialize the strategy before modifying the DAG.
  // This may initialize a DFSResult to be used for queue priority.
  SchedImpl->initialize(this);

  DEBUG(for (unsigned su = 0, e = SUnits.size(); su != e; ++su)
          SUnits[su].dumpAll(this));
  if (ViewMISchedDAGs) viewGraph();

  // Initialize ready queues now that the DAG and priority data are finalized.
  initQueues(TopRoots, BotRoots);

  bool IsTopNode = false;
  while (SUnit *SU = SchedImpl->pickNode(IsTopNode)) {
    assert(!SU->isScheduled && "Node already scheduled");
    if (!checkSchedLimit())
      break;

    scheduleMI(SU, IsTopNode);

    updateQueues(SU, IsTopNode);
  }
  assert(CurrentTop == CurrentBottom && "Nonempty unscheduled zone.");

  placeDebugValues();

  DEBUG({
      unsigned BBNum = begin()->getParent()->getNumber();
      dbgs() << "*** Final schedule for BB#" << BBNum << " ***\n";
      dumpSchedule();
      dbgs() << '\n';
    });
}
/// Build the DAG and setup three register pressure trackers.
// Don't worry about register pressure.
void ScheduleDAGMI::buildDAGWithRegPressure() {
  if (!ShouldTrackPressure) {
    RPTracker.reset();
    RegionCriticalPSets.clear();
    buildSchedGraph(AA);
    return;
  }

  // Initialize the register pressure tracker used by buildSchedGraph.
  RPTracker.init(&MF, RegClassInfo, LIS, BB, LiveRegionEnd,
                 /*TrackUntiedDefs=*/true);

  // Account for liveness generate by the region boundary.
  if (LiveRegionEnd != RegionEnd)
    RPTracker.recede();

  // Build the DAG, and compute current register pressure.
  buildSchedGraph(AA, &RPTracker, &SUPressureDiffs);

  // Initialize top/bottom trackers after computing region pressure.
  initRegPressure();
}

/// Move an instruction and update register pressure.
void ScheduleDAGMI::scheduleMI(SUnit *SU, bool IsTopNode) {
  // Move the instruction to its new location in the instruction stream.
  MachineInstr *MI = SU->getInstr();

  if (IsTopNode) {
    assert(SU->isTopReady() && "node still has unscheduled dependencies");
    if (&*CurrentTop == MI)
      CurrentTop = nextIfDebug(++CurrentTop, CurrentBottom);
    else {
      moveInstruction(MI, CurrentTop);
      TopRPTracker.setPos(MI);
    }

    if (ShouldTrackPressure) {
      // Update top scheduled pressure.
      TopRPTracker.advance();
      assert(TopRPTracker.getPos() == CurrentTop && "out of sync");
      updateScheduledPressure(SU, TopRPTracker.getPressure().MaxSetPressure);
    }
  }
  else {
    assert(SU->isBottomReady() && "node still has unscheduled dependencies");
    MachineBasicBlock::iterator priorII =
      priorNonDebug(CurrentBottom, CurrentTop);
    if (&*priorII == MI)
      CurrentBottom = priorII;
    else {
      if (&*CurrentTop == MI) {
        CurrentTop = nextIfDebug(++CurrentTop, priorII);
        TopRPTracker.setPos(CurrentTop);
      }
      moveInstruction(MI, CurrentBottom);
      CurrentBottom = MI;
    }
    if (ShouldTrackPressure) {
      // Update bottom scheduled pressure.
      SmallVector<unsigned, 8> LiveUses;
      BotRPTracker.recede(&LiveUses);
      assert(BotRPTracker.getPos() == CurrentBottom && "out of sync");
      updateScheduledPressure(SU, BotRPTracker.getPressure().MaxSetPressure);
      updatePressureDiffs(LiveUses);
    }
  }
}
/// Update scheduler queues after scheduling an instruction.
void ScheduleDAGMI::updateQueues(SUnit *SU, bool IsTopNode) {
  // Release dependent instructions for scheduling.
  if (IsTopNode)
    releaseSuccessors(SU);
  else
    releasePredecessors(SU);

  SU->isScheduled = true;

  if (DFSResult) {
    unsigned SubtreeID = DFSResult->getSubtreeID(SU);
    if (!ScheduledTrees.test(SubtreeID)) {
      ScheduledTrees.set(SubtreeID);
      DFSResult->scheduleTree(SubtreeID);
      SchedImpl->scheduleTree(SubtreeID);
    }
  }

  // Notify the scheduling strategy after updating the DAG.
  SchedImpl->schedNode(SU, IsTopNode);
}
