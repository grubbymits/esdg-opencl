#ifndef _LE1Simulator_H
#define _LE1Simulator_H

#include <pthread.h>

extern "C" { typedef struct systemConfig; }
extern "C" { typedef struct systemT; }


namespace Coal {
  class LE1Simulator {
  public:
    LE1Simulator();
    ~LE1Simulator();
    bool Initialise(const char *machine);
    void ClearRAM(void);
    int checkStatus(void);
    bool run(char* iram,
             char* dram);
    void readCharData(unsigned int addr,
                      unsigned int numBytes,
                      unsigned char *data);
    //unsigned short* readShortData(unsigned addr, unsigned numBytes);
    void readIntData(unsigned int addr,
                     unsigned int numBytes,
                     unsigned int* data);
private:
  pthread_mutex_t p_simulator_mutex;
  unsigned dram_size;
  unsigned IRAMFileSize;
  unsigned DRAMFileSize;
  systemConfig *SYS;
  systemT *system;
  };
}

#endif
