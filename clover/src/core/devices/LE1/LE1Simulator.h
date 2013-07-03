#ifndef _LE1Simulator_H
#define _LE1Simulator_H

#include <map>
#include <pthread.h>

extern "C" { typedef struct systemConfig; }
extern "C" { typedef struct systemT; }
extern "C" { typedef struct hyperContextT; }

namespace Coal {

  struct SimulationStats {
    SimulationStats(hyperContextT *HyperContext);
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
  };

  class LE1Simulator {

  public:
    LE1Simulator();
    ~LE1Simulator();
    bool Initialise(const char *machine);
    void SaveStats(void);
    std::vector<SimulationStats> *GetStats() { return &Stats; }
    void ClearStats() { Stats.clear(); }
    int checkStatus(void);
    bool Run();
    void LockAccess();
    void UnlockAccess();
    void readCharData(unsigned int addr,
                      unsigned int numBytes,
                      unsigned char *data);
    //unsigned short* readShortData(unsigned addr, unsigned numBytes);
    void readIntData(unsigned int addr,
                     unsigned int numBytes,
                     unsigned int* data);
private:
  static unsigned iteration;
  pthread_mutex_t p_simulator_mutex;
  unsigned dram_size;
  unsigned IRAMFileSize;
  unsigned DRAMFileSize;
  unsigned KernelNumber;
  systemConfig *SYS;
  systemT *LE1System;
  std::vector<SimulationStats> Stats;
  };

  typedef std::vector<SimulationStats> StatsSet;
  typedef std::map<std::string, StatsSet> StatsMap;
}

#endif
