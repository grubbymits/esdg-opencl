#ifndef _LE1Simulator_H
#define _LE1Simulator_H

#include <map>
#include <pthread.h>

extern "C" { struct systemConfig; }
extern "C" { struct systemT; }
extern "C" { struct hyperContextT; }

namespace Coal {

  struct SimulationStats {
    SimulationStats(hyperContextT *HyperContext,
                    unsigned dram, unsigned iram);
    SimulationStats(const SimulationStats &Stats);
    unsigned long TotalCycles;
    unsigned long Stalls;
    unsigned long NOPs;
    unsigned long IdleCycles;
    unsigned long DecodeStalls;
    unsigned long BranchesTaken;
    unsigned long BranchesNotTaken;
    unsigned long ControlFlowChange;
    unsigned long MemoryAccessCount;
    unsigned DramSize;
    unsigned IramSize;
  };

  typedef std::vector<SimulationStats> StatVector;
  typedef std::pair<unsigned, StatVector> StatSet;
  typedef std::map<std::string, std::pair<unsigned, StatSet> > StatsMap;

  class LE1Simulator {

  public:
    LE1Simulator();
    ~LE1Simulator();
    bool Initialise(const std::string &Machine);
    void Reset();
    void SaveStats(unsigned disabled);

    StatSet *GetStats() { return statSet; }
    unsigned GetIterations() const { return LE1Simulator::iteration; }

    void ClearStats() {
      statVector.clear();
      if (statSet)
        delete statSet;
    }

    int checkStatus(void);
    bool Run(const char *iram, const char *dram);//, const unsigned disabled);
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
    static unsigned iteration;
    std::string machineModel;
    bool isInitialised;
    pthread_mutex_t p_simulator_mutex;
    unsigned dram_size;
    unsigned IRAMFileSize;
    unsigned DRAMFileSize;
    unsigned KernelNumber;
    systemConfig *SYS;
    systemT *LE1System;
    StatVector statVector;
    StatSet *statSet;
  };

  //typedef std::vector<SimulationStats> StatsSet;
  //typedef std::map<std::string, StatsSet> StatsMap;
}

#endif
