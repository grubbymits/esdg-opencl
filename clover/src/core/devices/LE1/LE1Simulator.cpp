#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

LE1Simulator::LE1Simulator() {
  pthread_mutex_init(&p_simulator_mutex, 0);
  dram_size = 0;
}

LE1Simulator::~LE1Simulator() {
#ifdef DEBUGCL
  std::cerr << "! DELETING SIMULATOR !\n";
#endif
  pthread_mutex_destroy(&p_simulator_mutex);
  /* Clean up */
  if(freeMem() == -1) {
    fprintf(stderr, "!!! ERROR freeing memory !!!\n");
    exit(-1);
  }
}

bool LE1Simulator::Initialise(const char *machine) {

  LockAccess();
  /*
  if (pthread_mutex_lock(&p_simulator_mutex) != 0) {
    std::cerr << "!!! p_simulator_mutex lock failed !!!\n";
    exit(EXIT_FAILURE);
  }*/
  /* Setup global struct */
  /* readConf reads xml file and sets up global SYSTEM variable
     SYSTEM is a global pointer to systemConfig defined in inc/galaxyConfig.h
     This sets up the internal registers of the LE1 as defined in the VTPRM
  */
  if(readConf(const_cast<char*>(machine)) == -1) {
    fprintf(stderr, "!!! ERROR reading machine model file !!!\n");
    pthread_mutex_unlock(&p_simulator_mutex);
    return false;
  }

  /* Configure Insizzle structs */
  /* Using the SYSTEM registers defined above the registers and memories are setup
     Data structures are defined in inc/galaxy.h
     galaxyT is a global pointer of type systemT
  */
  if(setupGalaxy() == -1) {
    fprintf(stderr, "!!! ERROR setting up galaxy !!!\n");
    pthread_mutex_unlock(&p_simulator_mutex);
    return false;
  }

  /* Set the stack pointer and program counter of an available hypercontext */
  {
    //extern unsigned int STACK_SIZE; /* global var in Insizzle required for stack checking */

    /* system, context, hypercontext, cluster */
    unsigned char s = 0;
    unsigned char c = 0;
    unsigned char hc = 0;
    unsigned char cl = 0;

    SYS = (systemConfig *)((size_t)SYSTEM +
                                         (s * sizeof(systemConfig)));
    system = (systemT *)((size_t)galaxyT + (s * sizeof(systemT)));

    /* This is a value defined in the xml config file */
    // FIXME This needs to come from the device
    STACK_SIZE = SYS->STACK_SIZE;
    dram_size = ((SYS->DRAM_SHARED_CONFIG >> 8) & 0xFFFF) * 1024;
#ifdef DEBUGCL
    std::cerr << "DRAM_SHARED_CONFIG = " << SYS->DRAM_SHARED_CONFIG
      << std::endl;
#endif

    unsigned int totalHC = 0;
    /* loop through all contexts and hypercontexts */
    for (c=0; c<(SYS->SYSTEM_CONFIG & 0xff);  ++c){
      contextConfig *CNT = (contextConfig *)((size_t)SYS->CONTEXT +
                                             (c * sizeof(contextConfig)));
      contextT *context = (contextT *)((size_t)system->context
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

  //pthread_mutex_unlock(&p_simulator_mutex);
  UnlockAccess();
  return true;
}

void LE1Simulator::ClearRAM(void) {
#ifdef DEBUGCL
  std::cerr << "ClearRAM\n";
#endif

  /*
  if (pthread_mutex_lock(&p_simulator_mutex) != 0) {
    std::cerr << "!!! p_simulator_mutex lock failed !!!\n";
    exit(EXIT_FAILURE);
  }*/

  for (unsigned i = 0; (i < GALAXY_CONFIG & 0xFF); ++i) {

    SYS = (systemConfig *)((size_t)SYSTEM + (i * sizeof(systemConfig)));
    system = (systemT *)((size_t)galaxyT + (i * sizeof(systemT)));
    contextConfig *CNT
      = (contextConfig *)((size_t)SYS->CONTEXT + (0 * sizeof(contextConfig)));
    contextT* context =
      (contextT *)((size_t)system->context + (0 * sizeof(contextT)));

    for(unsigned k=0;k<((CNT->CONTEXT_CONFIG >> 4) & 0xf);k++) {
      hyperContextConfig *HCNT = (hyperContextConfig *)((size_t)CNT->HCONTEXT
                                    + (k * sizeof(hyperContextConfig)));
      hyperContextT *hypercontext =
        (hyperContextT *)((size_t)context->hypercontext
                          + (k * sizeof(hyperContextT)));
      memset(hypercontext->S_GPR, 0,
             (hypercontext->sGPRCount * sizeof(unsigned)));
      hypercontext->programCounter = 0;
    }
  }

  //pthread_mutex_unlock(&p_simulator_mutex);
#ifdef DEBUGCL
  std::cerr << "Leaving ClearRAM\n";
#endif
}

void LE1Simulator::LockAccess(void) {
#ifdef DEBUGCL
  std::cerr << "Trying to lock access to simulator\n";
#endif
  if (pthread_mutex_lock(&p_simulator_mutex) != 0) {
    std::cerr << "!!! p_simulator_mutex lock failed !!!\n";
    exit(EXIT_FAILURE);
  }
#ifdef DEBUGCL
  std::cerr << "Successfully locked access to simulator\n";
#endif
}

void LE1Simulator::UnlockAccess(void) {
#ifdef DEBUGCL
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

bool LE1Simulator::run(char* iram,
                       char* dram) {
#ifdef DEBUGCL
  std::cerr << "Entered LE1Simulator::run\n";
#endif
  /*
  if (pthread_mutex_lock(&p_simulator_mutex) != 0) {
    std::cerr << "!!! p_simulator_mutex lock failed !!!\n";
    exit(EXIT_FAILURE);
  }*/

  /* turn printout on */
  PRINT_OUT = 0;

    /* Load IRAM */
    {
#ifdef DEBUGCL
      std::cerr << "Loading IRAM\n";
#endif
      char *i_data = NULL;

      FILE *inst = fopen(iram, "rb");
      if(inst == NULL) {
        fprintf(stderr, "!!! ERROR Could not open file (%s) !!!\n", iram);
        pthread_mutex_unlock(&p_simulator_mutex);
        return false;
      }
      fseek(inst, 0L, SEEK_END);
      IRAMFileSize = ftell(inst);
      //printf("Filesize = %d\n", fileSize);
      fseek(inst, 0L, SEEK_SET);

      /* Create local data and copy content of file into it */
      i_data = (char *)calloc(sizeof(char), IRAMFileSize);
      if(i_data == NULL) {
        fprintf(stderr, "!!! ERROR Could not allocate memory (i_data) !!!\n");
        pthread_mutex_unlock(&p_simulator_mutex);
        return false;
      }
      fread(i_data, sizeof(char), IRAMFileSize, inst);
#ifdef DEBUGCL
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
#ifdef DEBUGCL
        std::cerr << "Got System Config\n";
#endif

        /* loop through all contexts and hypercontexts */
        for (c=0; c<(SYS->SYSTEM_CONFIG & 0xff); ++c) {
#ifdef DEBUGCL
          std::cerr << "Now looping through all contexts\n";
#endif
          contextConfig *CNT = (contextConfig *)((size_t)SYS->CONTEXT
                                                 + (c * sizeof(contextConfig)));
#ifdef DEBUGCL
          std::cerr << "Got Context Config\n";
#endif

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
              if(MSB2LSBDW(word) != (unsigned int)*(unsigned int *)(i_data + i)) {
                fprintf(stderr, "!!! ERROR: 0x%08x != 0x%08x !!!\n", word,
                        (unsigned int)*(unsigned int *)(i_data + i));
                pthread_mutex_unlock(&p_simulator_mutex);
                return false;
              }
            }
          }
        }
      }
    }
#ifdef DEBUGCL
      std::cerr << "Each context has the IRAM loaded\n";
#endif

    /* Free memory */
    free(i_data);
  }

  /* Load DRAM */
  {
#ifdef DEBUGCL
    std::cerr << "Loading DRAM\n";
#endif
    unsigned int DRAMFileSize = 0;
    char *d_data = NULL;

    FILE *data = fopen(dram, "rb");
    if(data == NULL) {
      fprintf(stderr, "!!! ERROR Could not open file (%s) !!!\n", dram);
      pthread_mutex_unlock(&p_simulator_mutex);
      return false;
    }
    fseek(data, 0L, SEEK_END);
    DRAMFileSize = ftell(data);
    fseek(data, 0L, SEEK_SET);

    if(DRAMFileSize > dram_size) {
      fprintf(stderr, "!!! ERROR DRAM is larger than the size specified !!!\n");
      fprintf(stderr, "DRAM = %d and fileSize = %d\n", dram_size, DRAMFileSize);
      pthread_mutex_unlock(&p_simulator_mutex);
      return false;
    }

    /* Create local data and copy content of file into it */
    d_data = (char *)calloc(sizeof(char), dram_size);
    if(d_data == NULL) {
      fprintf(stderr, "!!! ERROR Could not allocate memory (d_data) !!!\n");
      pthread_mutex_unlock(&p_simulator_mutex);
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
        pthread_mutex_unlock(&p_simulator_mutex);
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
#ifdef DEBUGCL
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
  {
    unsigned int vt_ctrl;
    cycleCount = 0;
    while(checkStatus()) {
      galaxyConfigT *g = NULL;
      gTracePacketT gTracePacket;
      memset((void *)&gTracePacket, 0, sizeof(gTracePacket)); /* clear gTracePacket memory */
      //printf("-------------------------------------------- end of cycle %lld\n",
        //     cycleCount);
      insizzleAPIClock(g, gTracePacket);

      /* Check here for control flow changes */
      /* Nothing in gTracePacket to find this */
      /* (inst.packet.newPCValid available in Insizzle for this) */

      insizzleAPIRdCtrl(&vt_ctrl);
      //printf("vt_ctrl = %d\n", vt_ctrl);
      ++cycleCount;
    }
  }

  //systemConfig *SYS =
    //(systemConfig *)((size_t)SYSTEM + (0 * sizeof(systemConfig)));
  /*
  if(memoryDump(((((SYS->DRAM_SHARED_CONFIG >> 8) & 0xffff) * 1024) >> 2), 0,
                system->dram) == -1) {
    pthread_mutex_unlock(&p_simulator_mutex);
    return false;
  }*/

  pthread_mutex_unlock(&p_simulator_mutex);

#ifdef DEBUGCL
  std::cerr << "Finished running simulation. globalS = "
    << std::hex << globalS << std::endl;
#endif
  return true;
}

void LE1Simulator::readCharData(unsigned int addr,
                                unsigned int numBytes,
                                unsigned char *data) {
  /*
  if (pthread_mutex_lock(&p_simulator_mutex) != 0) {
    std::cerr << "!!! p_simulator_mutex lock failed !!!\n";
    exit(EXIT_FAILURE);
  }*/
#ifdef DEBUGCL
  std::cerr << "Entering LE1Simulator::readCharData\n"
    << "Read from 0x" << std::hex << addr << ", " << std::dec << numBytes
    << " bytes. globalS = " << std::hex << globalS << std::endl;
#endif

  unsigned bytes = 0;
  if (numBytes < 4) {
    insizzleAPIRdOneDramLocation(addr, &bytes);
    //std::cout << "bytes = " << bytes << std::endl;
    for (unsigned i = 0; i < numBytes; ++i)
      data[i] = (unsigned char) (bytes >> (8 * (3-i)));
  }
  else if ((numBytes % 4) == 0) {
    for(unsigned i = 0; i < (numBytes >> 2); addr = (addr + 4), i += 4) {
      bytes = 0;
      insizzleAPIRdOneDramLocation(addr, &bytes);
      data[i+3] = (unsigned ) 0xFF & (bytes >> 0);
      data[i+2] = (unsigned ) 0xFF & (bytes >> 8);
      data[i+1] = (unsigned ) 0xFF & (bytes >> 16);
      data[i] = (unsigned ) 0xFF & (bytes >> 24);
      /*
      if (bytes != 0) {
        std::cout << std::hex << "bytes = " << bytes << " at address "
          << addr << std::endl;
        std::cout << "data[" << addr << " + 3] = " << (unsigned) data[i+3] << std::endl;
        std::cout << "data[" << addr << " + 2] = " << (unsigned) data[i+2] << std::endl;
        std::cout << "data[" << addr << " + 1] = " << (unsigned) data[i+1] << std::endl;
        std::cout << "data[" << addr << " + 0] = " << (unsigned) data[i+0] << std::endl;
      }*/
    }
  }

  //pthread_mutex_unlock(&p_simulator_mutex);
}
/*
short* LE1Simulator::readShortData(unsigned addr, unsigned numBytes) {
  for(unsigned i = addr; i < addr + numBytes; i=i+2) {

  }
}
*/
void LE1Simulator::readIntData(unsigned int addr,
                               unsigned int numBytes,
                               unsigned int* data) {
#ifdef DEBUGCL
  std::cerr << "Entering LE1Simulator::readIntData\n"
    << "Read from 0x" << std::hex << addr << ", " << std::dec << numBytes
    << " bytes.\n";
#endif
  /*
  if (pthread_mutex_lock(&p_simulator_mutex) != 0) {
    std::cerr << "!!! p_simulator_mutex lock failed !!!\n";
    exit(EXIT_FAILURE);
  }*/
  unsigned int num = numBytes >> 2;
  unsigned int word = 0;
  for(unsigned i = 0; i < num; addr = (addr + 4), ++i) {
#ifdef DEBUGCL
//    std::cerr << "Attempting a read from 0x" << std::hex << addr << std::endl;
#endif
    insizzleAPIRdOneDramLocation(addr, &word);
    data[i] = word;
#ifdef DEBUGCL
  //  std::cerr << "word = " << std::hex << word << std::endl;
#endif
  }

  //pthread_mutex_unlock(&p_simulator_mutex);
#ifdef DEBUGCL
  std::cerr << "Leaving LE1Simulator::readIntData\n";
#endif
}

