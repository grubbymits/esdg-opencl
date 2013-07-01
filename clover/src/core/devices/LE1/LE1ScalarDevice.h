#ifndef __LE1_SCALAR_DEVICE_H__
#define __LE1_SCALAR_DEVICE_H__

#include "LE1device.h"
#include "LE1Simulator.h"
#include "LE1worker.h"

#include <cstdlib>
#include <pthread.h>

namespace Coal {
  class LE1ScalarDevice : public LE1Device {
  public:
    LE1ScalarDevice() : LE1Device() {}
    bool init() {
      if (p_initialized)
        return true;

      pthread_cond_init(&p_events_cond, 0);
      pthread_mutex_init(&p_events_mutex, 0);
      p_workers = (pthread_t*) std::malloc(sizeof(pthread_t));
      pthread_create(&p_workers[0], 0, &worker, this);
      NumCores = 4;
      SimulatorModel = LE1Device::MachinesDir + "4Context_Default.xml";
      CompilerTarget = "scalar";
      if (!Simulator->Initialise(SimulatorModel.c_str()))
        return false;

      p_initialized = true;
      return true;
    }
  };
}

#endif
