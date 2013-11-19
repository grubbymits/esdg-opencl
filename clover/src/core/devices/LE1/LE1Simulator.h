#ifndef _LE1Simulator_H
#define _LE1Simulator_H

#define INSIZZLEAPI

#include <map>
#include <pthread.h>

extern "C" { struct systemConfig; }
extern "C" { struct systemT; }
extern "C" { struct hyperContextT; }

namespace Coal {

  struct SimulationStats {
    SimulationStats(hyperContextT *HyperContext, const unsigned disabled);
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
    unsigned disabledCores;
  };

  typedef std::vector<SimulationStats> StatsSet;
  typedef std::map<std::string, StatsSet> StatsMap;

  class LE1Simulator {

  public:
    LE1Simulator();
    ~LE1Simulator();
    bool Initialise(const std::string &Machine);
    void SaveStats(unsigned disabled);
    std::vector<SimulationStats> *GetStats() { return &Stats; }
    void ClearStats() { Stats.clear(); }
    int checkStatus(void);
    bool RunSim(const char *iram, const char *dram, const unsigned disabled);
    bool RunRTL(const char *iram, const char *dram);
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
  bool isInitialised;
  pthread_mutex_t p_simulator_mutex;
  unsigned dram_size;
  unsigned IRAMFileSize;
  unsigned DRAMFileSize;
  unsigned KernelNumber;
  systemConfig *SYS;
  systemT *LE1System;
  StatsSet Stats;
  };

  typedef std::vector<SimulationStats> StatsSet;
  typedef std::map<std::string, StatsSet> StatsMap;
}

#endif
