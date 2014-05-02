#ifndef _LE1Simulator_H
#define _LE1Simulator_H

#include <map>
#include <pthread.h>

extern "C" { struct systemConfig; }
extern "C" { struct systemT; }
extern "C" { struct hyperContextT; }

namespace Coal {

  struct ContextStats {
    ContextStats(hyperContextT *HyperContext);
    unsigned long TotalCycles;
    unsigned long Stalls;
    unsigned long NOPs;
    unsigned long IdleCycles;
    unsigned long DecodeStalls;
    unsigned long BranchesTaken;
    unsigned long BranchesNotTaken;
    unsigned long ControlFlowChange;
    unsigned long MemoryAccessCount;
  };

  struct IterationStats {
    IterationStats(unsigned iram, unsigned dram, unsigned long cycles);
    unsigned iram;
    unsigned dram;
    unsigned long completionCycles;
    std::vector<ContextStats*> contextStats;
  };

  typedef std::map<std::string, std::vector<IterationStats*> > KernelStats;

  class LE1Simulator {

  public:
    LE1Simulator();
    ~LE1Simulator();
    bool Initialise(const std::string &Machine);
    void Reset();
    void SaveStats(unsigned long cycles, std::string &kernel);

    const KernelStats *GetStats() const { return &kernelStats; }
    //unsigned GetIterations() const { return LE1Simulator::iteration; }

    //void ClearStats() {
      //statVector.clear();
      //if (statSet)
        //delete statSet;
    //}

    int checkStatus(void);
    bool Run(const char *iram, const char *dram, std::string &kernel);
    void LockAccess();
    void UnlockAccess();
    bool readByteData(unsigned int addr,
                      unsigned int numBytes,
                      unsigned char *data);
    bool readHalfData(unsigned addr,
                      unsigned numBytes,
                      unsigned short *data);
    bool readWordData(unsigned int addr,
                      unsigned int numBytes,
                      unsigned int* data);
private:
    //static unsigned iteration;
    std::string machineModel;
    bool isInitialised;
    pthread_mutex_t p_simulator_mutex;
    unsigned dram_size;
    unsigned IRAMFileSize;
    unsigned DRAMFileSize;
    unsigned KernelNumber;
    systemConfig *SYS;
    systemT *LE1System;
    //StatVector statVector;
    //StatSet *statSet;
    KernelStats kernelStats;
  };

  //typedef std::vector<SimulationStats> StatsSet;
  //typedef std::map<std::string, StatsSet> StatsMap;
}

#endif
