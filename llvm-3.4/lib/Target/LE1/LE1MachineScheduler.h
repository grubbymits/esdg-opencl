#include "llvm/CodeGen/MachineScheduler.h"
#include "llvm/CodeGen/RegisterClassInfo.h"
#include "llvm/CodeGen/ScheduleHazardRecognizer.h"

//===----------------------------------------------------------------------===//
// LE1Scheduler - Implementation of the generic MachineSchedStrategy.
//===----------------------------------------------------------------------===//

namespace llvm {

/// LE1SchedStrategy shrinks the unscheduled zone using heuristics to balance
/// the schedule.
class LE1SchedStrategy : public MachineSchedStrategy {
public:
  /// Represent the type of SchedCandidate found within a single queue.
  /// pickNodeBidirectional depends on these listed by decreasing priority.
  enum CandReason {
    NoCand, PhysRegCopy, RegExcess, RegCritical, Cluster, Weak, RegMax,
    ResourceReduce, ResourceDemand, BotHeightReduce, BotPathReduce,
    TopDepthReduce, TopPathReduce, NextDefUse, NodeOrder};

#ifndef NDEBUG
  static const char *getReasonStr(LE1SchedStrategy::CandReason Reason);
#endif

  /// Policy for scheduling the next instruction in the candidate's zone.
  struct CandPolicy {
    bool ReduceLatency;
    unsigned ReduceResIdx;
    unsigned DemandResIdx;

    CandPolicy(): ReduceLatency(false), ReduceResIdx(0), DemandResIdx(0) {}
  };

  /// Status of an instruction's critical resource consumption.
  struct SchedResourceDelta {
    // Count critical resources in the scheduled region required by SU.
    unsigned CritResources;

    // Count critical resources from another region consumed by SU.
    unsigned DemandedResources;

    SchedResourceDelta(): CritResources(0), DemandedResources(0) {}

    bool operator==(const SchedResourceDelta &RHS) const {
      return CritResources == RHS.CritResources
        && DemandedResources == RHS.DemandedResources;
    }
    bool operator!=(const SchedResourceDelta &RHS) const {
      return !operator==(RHS);
    }
  };

  /// Store the state used by LE1SchedStrategy heuristics, required for the
  /// lifetime of one invocation of pickNode().
  struct SchedCandidate {
    CandPolicy Policy;

    // The best SUnit candidate.
    SUnit *SU;

    // The reason for this candidate.
    CandReason Reason;

    // Set of reasons that apply to multiple candidates.
    uint32_t RepeatReasonSet;

    // Register pressure values for the best candidate.
    RegPressureDelta RPDelta;

    // Critical resource consumption of the best candidate.
    SchedResourceDelta ResDelta;

    SchedCandidate(const CandPolicy &policy)
      : Policy(policy), SU(NULL), Reason(NoCand), RepeatReasonSet(0) {}

    bool isValid() const { return SU; }

    // Copy the status of another candidate without changing policy.
    void setBest(SchedCandidate &Best) {
      assert(Best.Reason != NoCand && "uninitialized Sched candidate");
      SU = Best.SU;
      Reason = Best.Reason;
      RPDelta = Best.RPDelta;
      ResDelta = Best.ResDelta;
    }

    bool isRepeat(CandReason R) { return RepeatReasonSet & (1 << R); }
    void setRepeat(CandReason R) { RepeatReasonSet |= (1 << R); }

    void initResourceDelta(const ScheduleDAGMI *DAG,
                           const TargetSchedModel *SchedModel);
  };

  /// Summarize the unscheduled region.
  struct SchedRemainder {
    // Critical path through the DAG in expected latency.
    unsigned CriticalPath;
    unsigned CyclicCritPath;

    // Scaled count of micro-ops left to schedule.
    unsigned RemIssueCount;

    bool IsAcyclicLatencyLimited;

    // Unscheduled resources
    SmallVector<unsigned, 16> RemainingCounts;

    void reset() {
      CriticalPath = 0;
      CyclicCritPath = 0;
      RemIssueCount = 0;
      IsAcyclicLatencyLimited = false;
      RemainingCounts.clear();
    }

    SchedRemainder() { reset(); }

    void init(ScheduleDAGMI *DAG, const TargetSchedModel *SchedModel);
  };

  /// Each Scheduling boundary is associated with ready queues. It tracks the
  /// current cycle in the direction of movement, and maintains the state
  /// of "hazards" and other interlocks at the current cycle.
  struct SchedBoundary {
    ScheduleDAGMI *DAG;
    const TargetSchedModel *SchedModel;
    SchedRemainder *Rem;

    ReadyQueue Available;
    ReadyQueue Pending;
    bool CheckPending;

    // For heuristics, keep a list of the nodes that immediately depend on the
    // most recently scheduled node.
    SmallPtrSet<const SUnit*, 8> NextSUs;

    ScheduleHazardRecognizer *HazardRec;

    /// Number of cycles it takes to issue the instructions scheduled in this
    /// zone. It is defined as: scheduled-micro-ops / issue-width + stalls.
    /// See getStalls().
    unsigned CurrCycle;

    /// Micro-ops issued in the current cycle
    unsigned CurrMOps;

    /// MinReadyCycle - Cycle of the soonest available instruction.
    unsigned MinReadyCycle;

    // The expected latency of the critical path in this scheduled zone.
    unsigned ExpectedLatency;

    // The latency of dependence chains leading into this zone.
    // For each node scheduled bottom-up: DLat = max DLat, N.Depth.
    // For each cycle scheduled: DLat -= 1.
    unsigned DependentLatency;

    /// Count the scheduled (issued) micro-ops that can be retired by
    /// time=CurrCycle assuming the first scheduled instr is retired at time=0.
    unsigned RetiredMOps;

    // Count scheduled resources that have been executed. Resources are
    // considered executed if they become ready in the time that it takes to
    // saturate any resource including the one in question. Counts are scaled
    // for direct comparison with other resources. Counts can be compared with
    // MOps * getMicroOpFactor and Latency * getLatencyFactor.
    SmallVector<unsigned, 16> ExecutedResCounts;

    /// Cache the max count for a single resource.
    unsigned MaxExecutedResCount;

    // Cache the critical resources ID in this scheduled zone.
    unsigned ZoneCritResIdx;

    // Is the scheduled region resource limited vs. latency limited.
    bool IsResourceLimited;

#ifndef NDEBUG
    // Remember the greatest operand latency as an upper bound on the number of
    // times we should retry the pending queue because of a hazard.
    unsigned MaxObservedLatency;
#endif

    void reset() {
      // A new HazardRec is created for each DAG and owned by SchedBoundary.
      // Destroying and reconstructing it is very expensive though. So keep
      // invalid, placeholder HazardRecs.
      if (HazardRec && HazardRec->isEnabled()) {
        delete HazardRec;
        HazardRec = 0;
      }
      Available.clear();
      Pending.clear();
      CheckPending = false;
      NextSUs.clear();
      CurrCycle = 0;
      CurrMOps = 0;
      MinReadyCycle = UINT_MAX;
      ExpectedLatency = 0;
      DependentLatency = 0;
      RetiredMOps = 0;
      MaxExecutedResCount = 0;
      ZoneCritResIdx = 0;
      IsResourceLimited = false;
#ifndef NDEBUG
      MaxObservedLatency = 0;
#endif
      // Reserve a zero-count for invalid CritResIdx.
      ExecutedResCounts.resize(1);
      assert(!ExecutedResCounts[0] && "nonzero count for bad resource");
    }

    /// Pending queues extend the ready queues with the same ID and the
    /// PendingFlag set.
    SchedBoundary(unsigned ID, const Twine &Name):
      DAG(0), SchedModel(0), Rem(0), Available(ID, Name+".A"),
      Pending(ID << LE1SchedStrategy::LogMaxQID, Name+".P"),
      HazardRec(0) {
      reset();
    }

    ~SchedBoundary() { delete HazardRec; }

    void init(ScheduleDAGMI *dag, const TargetSchedModel *smodel,
              SchedRemainder *rem);

    bool isTop() const {
      return Available.getID() == LE1SchedStrategy::TopQID;
    }

#ifndef NDEBUG
    const char *getResourceName(unsigned PIdx) {
      if (!PIdx)
        return "MOps";
      return SchedModel->getProcResource(PIdx)->Name;
    }
#endif

    /// Get the number of latency cycles "covered" by the scheduled
    /// instructions. This is the larger of the critical path within the zone
    /// and the number of cycles required to issue the instructions.
    unsigned getScheduledLatency() const {
      return std::max(ExpectedLatency, CurrCycle);
    }

    unsigned getUnscheduledLatency(SUnit *SU) const {
      return isTop() ? SU->getHeight() : SU->getDepth();
    }

    unsigned getResourceCount(unsigned ResIdx) const {
      return ExecutedResCounts[ResIdx];
    }

    /// Get the scaled count of scheduled micro-ops and resources, including
    /// executed resources.
    unsigned getCriticalCount() const {
      if (!ZoneCritResIdx)
        return RetiredMOps * SchedModel->getMicroOpFactor();
      return getResourceCount(ZoneCritResIdx);
    }

    /// Get a scaled count for the minimum execution time of the scheduled
    /// micro-ops that are ready to execute by getExecutedCount. Notice the
    /// feedback loop.
    unsigned getExecutedCount() const {
      return std::max(CurrCycle * SchedModel->getLatencyFactor(),
                      MaxExecutedResCount);
    }

    bool checkHazard(SUnit *SU);

    unsigned findMaxLatency(ArrayRef<SUnit*> ReadySUs);

    unsigned getOtherResourceCount(unsigned &OtherCritIdx);

    void setPolicy(CandPolicy &Policy, SchedBoundary &OtherZone);

    void releaseNode(SUnit *SU, unsigned ReadyCycle);

    void bumpCycle(unsigned NextCycle);

    void incExecutedResources(unsigned PIdx, unsigned Count);

    unsigned countResource(unsigned PIdx, unsigned Cycles, unsigned ReadyCycle);

    void bumpNode(SUnit *SU);

    void releasePending();

    void removeReady(SUnit *SU);

    SUnit *pickOnlyChoice();

#ifndef NDEBUG
    void dumpScheduledState();
#endif
  };

private:
  const MachineSchedContext *Context;
  ScheduleDAGMI *DAG;
  const TargetSchedModel *SchedModel;
  const TargetRegisterInfo *TRI;

  // State of the top and bottom scheduled instruction boundaries.
  SchedRemainder Rem;
  SchedBoundary Top;
  SchedBoundary Bot;

  MachineSchedPolicy RegionPolicy;
public:
  /// SUnit::NodeQueueId: 0 (none), 1 (top), 2 (bot), 3 (both)
  enum {
    TopQID = 1,
    BotQID = 2,
    LogMaxQID = 2
  };

  LE1SchedStrategy(const MachineSchedContext *C):
    Context(C), DAG(0), SchedModel(0), TRI(0),
    Top(TopQID, "TopQ"), Bot(BotQID, "BotQ") {}

  virtual void initPolicy(MachineBasicBlock::iterator Begin,
                          MachineBasicBlock::iterator End,
                          unsigned NumRegionInstrs);

  bool shouldTrackPressure() const { return RegionPolicy.ShouldTrackPressure; }

  virtual void initialize(ScheduleDAGMI *dag);

  virtual SUnit *pickNode(bool &IsTopNode);

  virtual void schedNode(SUnit *SU, bool IsTopNode);

  virtual void releaseTopNode(SUnit *SU);

  virtual void releaseBottomNode(SUnit *SU);

  virtual void registerRoots();

protected:
  void checkAcyclicLatency();

  void tryCandidate(SchedCandidate &Cand,
                    SchedCandidate &TryCand,
                    SchedBoundary &Zone,
                    const RegPressureTracker &RPTracker,
                    RegPressureTracker &TempTracker);

  SUnit *pickNodeBidirectional(bool &IsTopNode);

  void pickNodeFromQueue(SchedBoundary &Zone,
                         const RegPressureTracker &RPTracker,
                         SchedCandidate &Candidate);

  void reschedulePhysRegCopies(SUnit *SU, bool isTop);

#ifndef NDEBUG
  void traceCandidate(const SchedCandidate &Cand);
#endif
};
} // namespace
