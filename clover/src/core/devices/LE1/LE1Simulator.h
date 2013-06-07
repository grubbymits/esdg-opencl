#ifndef _LE1Simulator_H
#define _LE1Simulator_H

#include <pthread.h>

namespace Coal {
  class LE1Simulator {
  public:
    LE1Simulator();
    ~LE1Simulator();
    bool Initialise(const char *machine);
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
  };
}

#endif
