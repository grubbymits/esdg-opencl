#include <iostream>

#include <stdio.h>
#include <cstdlib>
#include <string>
#include <sstream>
#include <vector>
#include <sys/mman.h>
#include "LE1device.h"
#include "LE1Simulator.h"

#define MSB2LSBDW( x )  (         \
                ( ( x & 0x000000FF ) << 24 ) \
                | ( ( x & 0x0000FF00 ) << 8 ) \
                | ( ( x & 0x00FF0000 ) >> 8 ) \
                | ( ( x & 0xFF000000 ) >> 24 ) \
                )
extern "C" {
#include <insizzle/functions.h>
#include <insizzle/xmlRead.h>
}

using namespace Coal;

//unsigned LE1Simulator::iteration = 0;

ContextStats::ContextStats(hyperContextT *HyperContext) {
  TotalCycles = HyperContext->cycleCount;
  Stalls = HyperContext->stallCount;
  NOPs = HyperContext->nopCount;
  IdleCycles = HyperContext->idleCount;
  DecodeStalls = HyperContext->decodeStallCount;
  BranchesTaken = HyperContext->branchTaken;
  BranchesNotTaken = HyperContext->branchNotTaken;
  ControlFlowChange = HyperContext->controlFlowChange;
  MemoryAccessCount = HyperContext->memoryAccessCount;
#ifdef DBG_OUTPUT
  std::cout << "Creating Stats:" << std::endl;
  std::cout << "TotalCycles = " << HyperContext->cycleCount << std::endl;
  std::cout << "Stalls = " << HyperContext->stallCount << std::endl;
  std::cout << "NOPs = " << HyperContext->nopCount << std::endl;
  std::cout << "IdleCycles = " << HyperContext->idleCount << std::endl;
  std::cout << "DecodeStalls = " << HyperContext->decodeStallCount << std::endl;
  std::cout << "BranchesTaken = " << HyperContext->branchTaken << std::endl;
  std::cout << "BranchesNotTaken = " << HyperContext->branchNotTaken
    << std::endl;
  std::cout << "ControlFlowChange = " << HyperContext->controlFlowChange
    << std::endl;
  std::cout << "MemoryAccessCount = " << HyperContext->memoryAccessCount
    << std::endl;
#endif

  // Reset the values
  HyperContext->cycleCount = 0;
  HyperContext->stallCount = 0;
  HyperContext->nopCount = 0;
  HyperContext->idleCount = 0;
  HyperContext->decodeStallCount = 0;
  HyperContext->branchTaken = 0;
  HyperContext->branchNotTaken = 0;
  HyperContext->controlFlowChange = 0;
  HyperContext->memoryAccessCount = 0;
}

IterationStats::IterationStats(unsigned iram, unsigned dram,
                               unsigned long cycles) {
  this->iram = iram;
  this->dram = dram;
  completionCycles = cycles;
}

/*
SimulationStats::SimulationStats(const SimulationStats &Stats) {
  TotalCycles = Stats.TotalCycles;
  Stalls = Stats.Stalls;
  NOPs = Stats.NOPs;
  IdleCycles = Stats.IdleCycles;
  DecodeStalls = Stats.DecodeStalls;
  BranchesTaken = Stats.BranchesTaken;
  BranchesNotTaken = Stats.BranchesNotTaken;
  ControlFlowChange = Stats.ControlFlowChange;
  MemoryAccessCount = Stats.MemoryAccessCount;
  DramSize = Stats.DramSize;
  IramSize = Stats.IramSize;
  //disabledCores = Stats.disabledCores;
}*/

LE1Simulator::LE1Simulator() {
  pthread_mutex_init(&p_simulator_mutex, 0);
  dram_size = 0;
  KernelNumber = 0;
  isInitialised = false;
  isLLVM = 1;
}

LE1Simulator::~LE1Simulator() {
#ifdef DBG_SIM
  std::cerr << "! DELETING SIMULATOR !\n";
#endif
  pthread_mutex_destroy(&p_simulator_mutex);

  /* Clean up */
  // FIXME Probably need to do this after each run
  if(freeMem() == -1) {
#ifdef DBG_OUTPUT
    std::cout << "!!! ERROR: trying to free memory in LE1Simulator"
      << std::endl;
#endif
    exit(-1);
  }

  for (KernelStats::iterator KSI = kernelStats.begin(),
       KSE = kernelStats.end(); KSI != KSE; ++KSI) {

    std::vector<IterationStats*> *iStats = &(KSI->second);
    for (std::vector<IterationStats*>::iterator ISI = iStats->begin(),
         ISE = iStats->end(); ISI != ISE; ++ISI) {

      (*ISI)->contextStats.clear();

    }
    iStats->clear();
  }
}

bool LE1Simulator::Initialise(const std::string &Machine) {
#ifdef DBG_SIM
  std::cerr << "Entering LE1Simulator::Initialise" << std::endl;
#endif

  LockAccess();

  if (isInitialised) {
#ifdef DBG_OUTPUT
    std::cout << "ERROR: Simulator is already initialised!\n";
#endif
    return false;
  }
  machineModel = Machine;
  /* Setup global struct */
  /* readConf reads xml file and sets up global SYSTEM variable
     SYSTEM is a global pointer to systemConfig defined in inc/galaxyConfig.h
     This sets up the internal registers of the LE1 as defined in the VTPRM
  */
  std::string FullPath = LE1Device::MachinesDir;
  FullPath.append(Machine);
  if(readConf(const_cast<char*>(FullPath.c_str())) == -1) {
#ifdef DBG_OUTPUT
    std::cout << "!!! ERROR reading machine model file : " << FullPath
      << std::endl;
#endif
    pthread_mutex_unlock(&p_simulator_mutex);
    return false;
  }

  /* Configure Insizzle structs */
  /* Using the SYSTEM registers defined above the registers and memories are setup
     Data structures are defined in inc/galaxy.h
     galaxyT is a global pointer of type systemT
  */
  if(setupGalaxy() == -1) {
#ifdef DBG_OUTPUT
    std::cout << "!!! ERROR setting up galaxy" << std::endl;
#endif
    //fprintf(stderr, "!!! ERROR setting up galaxy !!!\n");
    pthread_mutex_unlock(&p_simulator_mutex);
    return false;
  }

  Reset();

  isInitialised = true;

  //pthread_mutex_unlock(&p_simulator_mutex);
  UnlockAccess();
  return true;
}

void LE1Simulator::Reset() {
  /* Set the stack pointer and program counter of an available hypercontext */

  /* system, context, hypercontext, cluster */
  unsigned char s = 0;
  unsigned char c = 0;
  unsigned char hc = 0;
  unsigned char cl = 0;

  SYS = (systemConfig *)((size_t)SYSTEM +
                                       (s * sizeof(systemConfig)));
  LE1System = (systemT *)((size_t)galaxyT + (s * sizeof(systemT)));

  /* This is a value defined in the xml config file */
  // FIXME This needs to come from the device
  STACK_SIZE = SYS->STACK_SIZE;
  dram_size = ((SYS->DRAM_SHARED_CONFIG >> 8) & 0xFFFF) * 1024;
#ifdef DBG_SIM
  std::cerr << "dram_size = " << dram_size << std::endl;
#endif

  unsigned int totalHC = 0;
  /* loop through all contexts and hypercontexts */
  for (c=0; c<(SYS->SYSTEM_CONFIG & 0xff);  ++c){
    contextConfig *CNT = (contextConfig *)((size_t)SYS->CONTEXT +
                                           (c * sizeof(contextConfig)));
    contextT *context = (contextT *)((size_t)LE1System->context
                                     + (c * sizeof(contextT)));

    for (hc=0; hc<((CNT->CONTEXT_CONFIG >> 4) & 0xf); ++hc) {
      hyperContextT *hypercontext =
        (hyperContextT *)((size_t)context->hypercontext
                          + (hc * sizeof(hyperContextT)));

      /* Set current hypercontext to interact with */
      insizzleAPISetCurrent(s, c, hc, cl); /* (system, context, hypercontext, cluster) */
      /* Write stack pointer (SGPR 1) */
      insizzleAPIWrOneSGpr(1, (dram_size - 256) - ((STACK_SIZE * 1024) * totalHC));
      /* Set original stack pointer and stack size for Insizzle to perform stack check */
      hypercontext->initialStackPointer
        = (dram_size - 256) - ((STACK_SIZE * 1024) * totalHC);

      /* Write Program Counter */
      insizzleAPIWrPC(0x0);

      ++totalHC;
    }
  }
}

// Save the execution statistics and resets the device
void LE1Simulator::SaveStats(unsigned long cycles, std::string &kernel) {
  SYS = (systemConfig *)((size_t)SYSTEM + (0 * sizeof(systemConfig)));
  LE1System = (systemT *)((size_t)galaxyT + (0 * sizeof(systemT)));

  // Record the completion cycles and ram sizes for this iteration.
  IterationStats *iterationStats = new IterationStats(IRAMFileSize,
                                                      DRAMFileSize,
                                                      cycles);

  // Then populate the iteration stats with the individual stats from all the
  // available contexts.
  for (unsigned c = 0; c < (SYS->SYSTEM_CONFIG & 0xFF); ++c) {
    contextConfig *CNT
      = (contextConfig *)((size_t)SYS->CONTEXT + (c * sizeof(contextConfig)));
    contextT* context =
      (contextT *)((size_t)LE1System->context + (c * sizeof(contextT)));


    for(unsigned k=0;k<((CNT->CONTEXT_CONFIG >> 4) & 0xf);k++) {

      hyperContextT *hypercontext =
        (hyperContextT *)((size_t)context->hypercontext
                          + (k * sizeof(hyperContextT)));

      // Save the execution statistics
      ContextStats *contextStat = new ContextStats(hypercontext);
      iterationStats->contextStats.push_back(contextStat);
    }
  }

  // Create a new item in the map if this is the first time the kernel has
  // ran.
  if (kernelStats.find(kernel) == kernelStats.end()) {
    std::vector<IterationStats*> stats;
    stats.push_back(iterationStats);
    kernelStats.insert(std::make_pair(kernel, stats));
  }
  else
    kernelStats[kernel].push_back(iterationStats);

  Reset();
}

void LE1Simulator::LockAccess(void) {
#ifdef DBG_SIM
  std::cerr << "Trying to lock access to simulator\n";
#endif
  if (pthread_mutex_lock(&p_simulator_mutex) != 0) {
#ifdef DBG_OUTPUT
    std::cout << "!!! p_simulator_mutex lock failed !!!\n";
#endif
    exit(EXIT_FAILURE);
  }
#ifdef DBG_SIM
  std::cerr << "Successfully locked access to simulator\n";
#endif
}

void LE1Simulator::UnlockAccess(void) {
#ifdef DBG_SIM
  std::cerr << "Unlocking access to simulator\n";
#endif
  pthread_mutex_unlock(&p_simulator_mutex);
}


/* Loop through available hypercontexts checking VT_CTRL register */
int LE1Simulator::checkStatus(void) {
  /* system, context, hypercontext, cluster */
  unsigned char s = 0;
  unsigned char c = 0;
  unsigned char hc = 0;
  unsigned char cl = 0;

  //systemConfig *SYS = (systemConfig *)((size_t)SYSTEM + (s * sizeof(systemConfig)));

  /* loop through all contexts and hypercontexts */
  for (c=0; c<(SYS->SYSTEM_CONFIG & 0xff);  ++c){
    contextConfig *CNT = (contextConfig *)((size_t)SYS->CONTEXT +
                                           (c * sizeof(contextConfig)));

    for (hc=0;  hc<((CNT->CONTEXT_CONFIG >> 4) & 0xf);  ++hc) {
      unsigned int vt_ctrl;

      /* Set current hypercontext to interact with */
      insizzleAPISetCurrent(s, c, hc, cl); /* (system, context, hypercontext, cluster) */

      insizzleAPIRdCtrl(&vt_ctrl);
      if(vt_ctrl != DEBUG) {
        return 1;
      }
    }
  }
  return 0;
}

bool LE1Simulator::Run(const char *iram, const char *dram,
                       std::string &kernel) {
#ifdef DBG_SIM
  std::cerr << "Entered LE1Simulator::run with:\n" << iram << std::endl << dram
    << std::endl << machineModel << std::endl;
#endif
#ifdef DBG_OUTPUT
  std::cout << "Run Simulation with:\n  " << iram << "  " << std::endl
    << "  " << dram << "  " << std::endl << "  " << machineModel << std::endl;
#endif
  //LockAccess();
  ++KernelNumber;

  /* turn printout on */
  PRINT_OUT = true;
  bool MEM_DUMP = false;

    /* Load IRAM */
    {
#ifdef DBG_SIM
      std::cerr << "Loading IRAM\n";
#endif
      char *i_data = NULL;

      FILE *inst = fopen(iram, "rb");
      if(inst == NULL) {
#ifdef DBG_OUTPUT
        std::cout << "!!! ERROR: Could not open file " << iram << std::endl;
#endif
        //pthread_mutex_unlock(&p_simulator_mutex);
        return false;
      }
      fseek(inst, 0L, SEEK_END);
      IRAMFileSize = ftell(inst);
      //printf("Filesize = %d\n", fileSize);
#ifdef DBG_OUTPUT
    std::cout << "IRAMFileSize = " << IRAMFileSize << std::endl;
#endif
      fseek(inst, 0L, SEEK_SET);

      /* Create local data and copy content of file into it */
      i_data = (char *)calloc(sizeof(char), IRAMFileSize);
      if(i_data == NULL) {
#ifdef DBG_OUTPUT
        std::cout << "!!! ERROR: Could not allocate memory for i_data"
          << std::endl;
#endif
        //pthread_mutex_unlock(&p_simulator_mutex);
        return false;
      }
      fread(i_data, sizeof(char), IRAMFileSize, inst);
#ifdef DBG_SIM
      std::cerr << "Read in IRAM file\n";
#endif

      /* Input into each available iram */
      {
        /* system, context, hypercontext, cluster */
        unsigned char s = 0;
        unsigned char c = 0;
        unsigned char hc = 0;
        unsigned char cl = 0;

        //systemConfig *SYS = (systemConfig *)((size_t)SYSTEM
          //                                   + (s * sizeof(systemConfig)));
#ifdef DBG_SIM
        std::cerr << "Got System Config, " << (SYS->SYSTEM_CONFIG & 0xff)
          << " contexts\n";
#endif

        /* loop through all contexts and hypercontexts */
        for (c=0; c<(SYS->SYSTEM_CONFIG & 0xff); ++c) {
          contextConfig *CNT = (contextConfig *)((size_t)SYS->CONTEXT
                                                 + (c * sizeof(contextConfig)));

          for (hc=0; hc<((CNT->CONTEXT_CONFIG >> 4) & 0xf); ++hc) {
            /* Set current hypercontext to interact with */
            insizzleAPISetCurrent(s, c, hc, cl); /* (system, context, hypercontext, cluster) */
            /* Load Iram into current hypercontext */
            insizzleAPILdIRAM(i_data, IRAMFileSize);

            {
            /* Check correct */
            unsigned int i;
            unsigned int word;
            for (i=0; i<IRAMFileSize; i+=4) {
              insizzleAPIRdOneIramLocation(i, &word);
              /* Need to perform an endian flip */
              if(MSB2LSBDW(word) != (unsigned int)*(unsigned int *)(i_data + i))
              {
#ifdef DBG_OUTPUT
                std::cout << "!! ERROR: " << word << " != " <<
                  (unsigned*)(unsigned*)(i_data + i) << std::endl;
#endif
                return false;
              }
            }
          }
        }
      }
    }
#ifdef DBG_SIM
      std::cerr << "Each context has the IRAM loaded\n";
#endif

    /* Free memory */
    free(i_data);
  }

  /* Load DRAM */
  {
#ifdef DBG_SIM
    std::cerr << "Loading DRAM\n";
#endif
    char *d_data = NULL;

    FILE *data = fopen(dram, "rb");
    if(data == NULL) {
#ifdef DBG_OUTPUT
      std::cout << "!!! ERROR: Could not open file - " << dram << std::endl;
#endif
      //pthread_mutex_unlock(&p_simulator_mutex);
      return false;
    }
    fseek(data, 0L, SEEK_END);
    DRAMFileSize = ftell(data);
    fseek(data, 0L, SEEK_SET);
#ifdef DBG_SIM
    std::cerr << "Machine DRAM = " << dram_size << ", total used = "
      << DRAMFileSize << std::endl;
#endif
#ifdef DBG_OUTPUT
    std::cout << "DRAMFileSize = " << DRAMFileSize << std::endl;
#endif

    if(DRAMFileSize > dram_size) {
#ifdef DBG_OUTPUT
      std::cout << "!!! ERROR DRAM is larger than the size specified !!!\n"
        << "DRAM = " << dram_size << " and fileSize = " << DRAMFileSize
        << std::endl;
#endif
      //pthread_mutex_unlock(&p_simulator_mutex);
      return false;
    }

    /* Create local data and copy content of file into it */
    d_data = (char *)calloc(sizeof(char), dram_size);
    if(d_data == NULL) {
#ifdef DBG_OUTPUT
      std::cout << "!!! ERROR: Could not allocate memory for d_data"
        << std::endl;
#endif
      //pthread_mutex_unlock(&p_simulator_mutex);
      return false;
    }
    fread(d_data, sizeof(char), DRAMFileSize, data);

    /* Input into dram */
    /* Send size dram  to allocate */
    insizzleAPILdDRAM(d_data, dram_size);

    /* Check correct */
    unsigned int i;
    unsigned int word;
    for(i=0;i<dram_size;i+=4) {
      insizzleAPIRdOneDramLocation(i, &word);
      /* Need to perform an endian flip */
      if(MSB2LSBDW(word) != (unsigned int)*(unsigned int *)(d_data + i)) {
        fprintf(stderr, "!!! ERROR: 0x%08x != 0x%08x !!!\n", word,
                (unsigned int)*(unsigned int *)(d_data + i));
        //pthread_mutex_unlock(&p_simulator_mutex);
        return false;
      }
    }
    /* Free memory */
    free(d_data);
  }

  /* Use WrCtrl to put hypercontext in various modes
     see vtapi.h and vtprm for modes and their descriptions
     DEBUG, RUNNING and READY should be most important
     DEBUG - hypercontext stalled, this is the state hypercontext hits when a HALT inst is issued
     RUNNING - hypercontext is current executing instructions
     READY - hypercontext is ready to execute
  */

  /* Set all available Context to RUNNING */
  {
#ifdef DBG_SIM
    std::cerr << "Setting Contexts to RUNNING\n";
#endif
  /* system, context, hypercontext, cluster */
    unsigned char s = 0;
    unsigned char c = 0;
    unsigned char hc = 0;
    unsigned char cl = 0;

    //systemConfig *SYS = (systemConfig *)((size_t)SYSTEM + (s * sizeof(systemConfig)));

    /* loop through all contexts and hypercontexts */
    for (c=0; c<(SYS->SYSTEM_CONFIG & 0xff); ++c){
      contextConfig *CNT = (contextConfig *)((size_t)SYS->CONTEXT
                                             + (c * sizeof(contextConfig)));

      for (hc=0; hc<((CNT->CONTEXT_CONFIG >> 4) & 0xf); ++hc) {
        /* Set current hypercontext to interact with */
        insizzleAPISetCurrent(s, c, hc, cl); /* (system, context, hypercontext, cluster) */

         /* vtCtrlStatE enum defined in vtapi.h */
         insizzleAPIWrCtrl(RUNNING);
         //printf("%d %d %d %d %d\n", s,c,hc,cl, RUNNING);
      }
    }
  }


  /* Ready back the vt_ctrl register */
  {
    /* system, context, hypercontext, cluster */
    unsigned char s = 0;
    unsigned char c = 0;
    unsigned char hc = 0;
    unsigned char cl = 0;

    //systemConfig *SYS = (systemConfig *)((size_t)SYSTEM
      //                                   + (s * sizeof(systemConfig)));

    /* loop through all contexts and hypercontexts */
    for (c=0; c<(SYS->SYSTEM_CONFIG & 0xff); ++c) {
      contextConfig *CNT = (contextConfig *)((size_t)SYS->CONTEXT
                                             + (c * sizeof(contextConfig)));

      for (hc=0; hc<((CNT->CONTEXT_CONFIG >> 4) & 0xf); ++hc) {
        unsigned int val;

        /* Set current hypercontext to interact with */
        insizzleAPISetCurrent(s, c, hc, cl); /* (system, context, hypercontext, cluster) */

        insizzleAPIRdCtrl(&val);
        //printf("vt_ctrl: %d\n", val);
      }
    }
  }


  //extern unsigned long long cycleCount;
  /* Clock */
  cycleCount = 0;
  {
    unsigned int vt_ctrl;
    while(checkStatus()) {
      galaxyConfigT *g = NULL;
      gTracePacketT gTracePacket;
      memset((void *)&gTracePacket, 0, sizeof(gTracePacket)); /* clear gTracePacket memory */
      if (PRINT_OUT)
        printf("-------------------------------------------- end of cycle %lld\n",
          cycleCount);

      if (insizzleAPIClock(g, gTracePacket) == INSIZZLE_FAIL) {
        // Something has gone wrong...
        printf("-------------------------------------------- end of cycle %lld\n",
          cycleCount);
#ifdef DBG_OUTPUT
        std::cout << "!!! ERROR - Simulation failed!" << std::endl;
#endif
        return false;
      }


      /* Check here for control flow changes */
      /* Nothing in gTracePacket to find this */
      /* (inst.packet.newPCValid available in Insizzle for this) */

      insizzleAPIRdCtrl(&vt_ctrl);
      //printf("vt_ctrl = %d\n", vt_ctrl);
      ++cycleCount;
    }
  }

  // Debugging information
  if (MEM_DUMP) { 
    if(memoryDump(((((SYS->DRAM_SHARED_CONFIG >> 8) & 0xffff) * 1024) >> 2), 0,
                  LE1System->dram) == -1) {
      //pthread_mutex_unlock(&p_simulator_mutex);
      return false;
    }
  }

#ifdef DBG_OUTPUT
  std::cout << "Simulation finished, cycleCount = " << cycleCount << std::endl;
  std::cout << " -------------------------------------------------------- "
    << std::endl;
#endif
  SaveStats(cycleCount, kernel);

  //pthread_mutex_unlock(&p_simulator_mutex);

#ifdef DBG_SIM
  std::cerr << "Finished running simulation." << std::endl;
#endif
  return true;
}

bool LE1Simulator::readByteData(unsigned int addr,
                                unsigned int numBytes,
                                unsigned char *data) {
#ifdef DBG_SIM
  std::cerr << "Entering LE1Simulator::readCharData\n"
    << "Read from 0x" << std::hex << addr << ", " << std::dec << numBytes
    << " bytes." << std::endl;
#endif

  unsigned bytes = 0;
  if (numBytes < 4) {
    insizzleAPIRdOneDramLocation(addr, &bytes);
    for (unsigned i = 0; i < numBytes; ++i)
      data[i] = (unsigned char) (bytes >> (8 * (3-i)));
  }
  else {
    unsigned remainingBytes = numBytes % 4;
    unsigned lastPos = 0;
    for (unsigned i = 0; i < (numBytes - remainingBytes);
         addr = (addr + 4), i += 4) {
      bytes = 0;
      insizzleAPIRdOneDramLocation(addr, &bytes);
      data[i+3] = (unsigned char) 0xFF & (bytes >> 0);
      data[i+2] = (unsigned char) 0xFF & (bytes >> 8);
      data[i+1] = (unsigned char) 0xFF & (bytes >> 16);
      data[i] = (unsigned char) 0xFF & (bytes >> 24);
      lastPos = i + 4;
    }
    if (remainingBytes != 0) {
      bytes = 0;
      insizzleAPIRdOneDramLocation(addr, &bytes);
      for (unsigned i = 0; i < remainingBytes; ++i)
        data[lastPos + i] = 0xFF & (bytes >> (24 - (i * 8)));
    }
  }

  return true;
  //pthread_mutex_unlock(&p_simulator_mutex);
}

bool LE1Simulator::readHalfData(unsigned addr,
                                unsigned numBytes,
                                unsigned short *data) {
  unsigned bytes = 0;
  if (numBytes < 4) {
    insizzleAPIRdOneDramLocation(addr, &bytes);
    data[0] = (unsigned short) (bytes >> 16);
  }
  else if ((numBytes % 4) == 0) {
    for(unsigned i = addr; i < addr + numBytes; i=i+2) {
      bytes = 0;
      insizzleAPIRdOneDramLocation(addr, &bytes);
      data[i+1] = (unsigned short) 0xFF & bytes;
      data[i] = (unsigned short) 0xFF & (bytes >> 16);
    }
  }
  else {
#ifdef DBG_OUTPUT
    std::cout << "!!! ERROR: Unhandled number of bytes to read back from device"
      << std::endl;
#endif
    return false;
  }
  return true;
}

bool LE1Simulator::readWordData(unsigned int addr,
                                unsigned int numBytes,
                                unsigned int* data) {
#ifdef DBG_SIM
  std::cerr << "Entering LE1Simulator::readIntData\n"
    << "Read from 0x" << std::hex << addr << ", " << std::dec << numBytes
    << " bytes.\n";
#endif
  unsigned int num = numBytes >> 2;
  unsigned int word = 0;
  for(unsigned i = 0; i < num; addr = (addr + 4), ++i) {
    insizzleAPIRdOneDramLocation(addr, &word);
#ifdef DBG_SIM
    /*
    if (data[i] != word)
      std::cout << "updating " << std::hex << addr << " with different data: "
        << "current data = " << std::dec << (int)data[i] << " and new = "
        << (int)word << std::endl;*/
#endif
    data[i] = word;
  }

  //pthread_mutex_unlock(&p_simulator_mutex);
#ifdef DBG_SIM
  std::cerr << "Leaving LE1Simulator::readIntData\n";
#endif
  return true;
}

