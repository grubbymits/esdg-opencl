/************************************************************************************************
 * driver.c
 * Code example to use the VT API
 * (C) 2010 V. Chouliaras
 * Proprietary and Confidential
 * vassilios@vassilios-chouliaras.com
 ************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "vtapi.h"

#undef SCTB
#ifdef SCTB
#include <systemc.h>
//#include "/home/elvc/projects/vt/rtl/vc_vt_top.h"
#include "sctb.h"
#define HOST X86
#define TARGET RTL

#else
#ifdef FPGA_BOARD_STANDALONE
#define HOST MICROBLAZE
#define TARGET LE1_SILICON
#else
#ifdef FPGA_BOARD_HOST
#define HOST X86
#define TARGET LE1_SILICON
#endif
#endif
#endif



// CPUIDTEST
#include "cpuidtest.inst.forXilinx.h"
#include "cpuidtest.data.forXilinx.h"
#include "cpuidtest_le1_objects.h"
// OREMARK_NO_STDIO
//#include "/home/elvc/projects/benchmarks/coremark_no_stdio/COREMARK_NO_STDIO/COREMARK_NO_STDIO.inst.forXilinx.h"
//#include "/home/elvc/projects/benchmarks/coremark_no_stdio/COREMARK_NO_STDIO/COREMARK_NO_STDIO.data.forXilinx.h"
//#include "/home/elvc/projects/benchmarks/coremark_no_stdio/COREMARK_NO_STDIO/COREMARK_NO_STDIO_le1_objects.h"
// HANOI
//#include "/home/elvc/projects/benchmarks/hanoi/hanoi/hanoi.inst.forXilinx.h"
//#include "/home/elvc/projects/benchmarks/hanoi/hanoi/hanoi.data.forXilinx.h"
//#include "/home/elvc/projects/benchmarks/hanoi/hanoi/hanoi_le1_objects.h"
// 3GPP
//#include "/home/elvc/projects/benchmarks/threegpp/THREEGPP/THREEGPP.inst.forXilinx.h"
//#include "/home/elvc/projects/benchmarks/threegpp/THREEGPP/THREEGPP.data.forXilinx.h"
//#include "/home/elvc/projects/benchmarks/threegpp/THREEGPP/THREEGPP_le1_objects.h"
#define NCONTEXTS 2
#define CONTEXTOFFS 0
extern int vtApiInit (galaxyConfigT * galaxyConfig , hostE host , targetE target);

#ifdef SCTB

#include "sctb.h"
#include <iostream>


SC_MODULE_EXPORT(sctb);

#endif

//int main1(int argc , char * argv[])
#ifdef SCTB
void driver(void)
#else
  int driver_cpuidtest(void)
#endif
{
  //{
  /*
  //printf ("generic driver\n");
  //
  printf ("Usage:%s\n",usage);
  if (argc < 2) {
  printf ("Usage:%s\n",usage);
  exit(0);
  }
  else
  */
  {
    /*#ifdef SCTB
      hostE host=X86;
      targetE target=SYSTEMCTB;
      #else
      Specify target
      hostE host = MB;
      targetE target=SILICON;
      #endif*/
    hostE host = MB;
    targetE target=SILICON;

    int err;	// error code
    /* Cant specify multiple targets like SIMSTUB | INSIZLE | SILICON
       target = target | SIMSTUB; Instead, use multiple initializers */
    /* Ptr to array holding the memory dump after execution */
    int * memDump;
    int memDumpStatic[128 * 1024];

    // temp var holding the DRAM memory size (in KB)
    int dramSize;
    /* Ptr to architected state */
    oneArchStateT * oneArchStateDump;
    oneArchStateT  oneArchStateDumpStatic;

    /* ######################################
     * Step_0 : Create and initialize the galaxyConfig structure. This is used to hold all configuration
     * and state information regarding the target processor.
     ######################################## */
    galaxyConfigT galaxyConfig;

    printf ("generic driver\n");
    //almabench_main("-ga");
    //goto a1;
    if (err=vtApiInit( & galaxyConfig , host , target )) exit (err);

    printf ("galaxyConfig initialized.\n");
    //exit(0);

    //printGalaxyConfig (&galaxyConfig);

    /* ######################################
     * Step_1a :
     * Allocate the memory for the SYSTEM
     * memory dump
     ######################################## */
    //dramSize= (((galaxyConfig.ctrlState.dram_shared_config0_reg[0]) >> DRAM_SHARED_CONFIG0_REG_DRAM_SIZE_SHIFT) & DRAM_SHARED_CONFIG0_REG_DRAM_SIZE_MASK);

    printf ("dramSize=%dK\n",dramSize);
    //if ((memDump=(int*) (malloc (sizeof(char) *  dramSize)  == 0)) {
    //if ((memDump=(int*) malloc ((sizeof(char) *  1024 * dramSize))  == 0)) {
    //memDump=	(int*) malloc (sizeof(char) *  dramSize*1024);
    memDump=	(int*) malloc (1024);
    //memDump=
    if ((memDump == 0)) {
      printf ("Unable to allocate dramSize (memDump=%x)\n",memDump);
      //exit (FAILURE);
    }
    printf ("memDump initialized. Allocated %d KBs of memory\n",dramSize);

    /* ######################################
     * Step_1b :
     * Allocate the memory for the ARCHSTATE
     * dump
     ######################################## */
    if ((oneArchStateDump=(oneArchStateT*) malloc ((sizeof(oneArchStateT))))==0) {
      printf ("Unable to allocate oneArchStateDump (oneArchStateDump=%x)\n",oneArchStateDump);
      //exit (FAILURE);
    }
    printf ("oneArchStateDump initialized\n");
  a1:
    ;

    //testlink();
    int iter,c;

#define ITER 1
#define SPOFFSET 0x1000
    for (iter=0;iter<ITER;iter++) {
      //goto skip_processing;
      /* ######################################
       * Step_1a :
       * Use vtApiForkMultiCHC0
       ######################################## */
      if (err=
	  vtApiForkMultiCHC0(	&galaxyConfig,	/* galaxyConfigT galaxyConfig */
				0, 				/*int system 	*/
				NCONTEXTS, 				/*int contexts 	*/
				CONTEXTOFFS,		/* context offset */
				0x0,			/* pc */
				SPOFFSET	,		/* spOffset */

				cpuidtest_i, 			/*char * iramBin*/
				CPUIDTEST_INST_SIZE, 	/*int iramLen 	*/
				cpuidtest_d, 			/*char * dramBin*/
				CPUIDTEST_DATA_SIZE, 	/*int dramLen 	*/
				//
				//   					COREMARK_NO_STDIO_i, 			/*char * iramBin*/
				//   					COREMARK_NO_STDIO_INST_SIZE, 	/*int iramLen 	*/
				//   					COREMARK_NO_STDIO_d, 			/*char * dramBin*/
				//   					COREMARK_NO_STDIO_DATA_SIZE, 	/*int dramLen 	*/

				//   					THREEGPP_i, 			/*char * iramBin*/
				//   					THREEGPP_INST_SIZE, 	/*int iramLen 	*/
				//   					THREEGPP_d, 			/*char * dramBin*/
				//   					THREEGPP_DATA_SIZE, 	/*int dramLen 	*/
				//   					hanoi_i, 			/*char * iramBin*/
				//   					HANOI_INST_SIZE, 	/*int iramLen 	*/
				//   					hanoi_d, 			/*char * dramBin*/
				//   					HANOI_DATA_SIZE, 	/*int dramLen 	*/
				host,			/* hostE host	*/
				target 			/*targetE target*/))
	exit (FAILURE);
      printf ("vtApiForkMultiCHC0 succesfull\n");

      /* ######################################
       * Step_1a :
       * Use vtApiJoinMultiCHC0
       ######################################## */
      if (err=
	  vtApiJoinMultiCHC0(	&galaxyConfig,	/* galaxyConfigT galaxyConfig */
				0, 				/*int system 	*/
				NCONTEXTS, 				/*int contexts 	*/
				CONTEXTOFFS,		/* context offset */
				host,			/* hostE host	*/
				target 			/*targetE target*/))
	exit (FAILURE);
      printf ("vtApiJoinMultiCHC0 succesfull\n");

    skip_processing:

      /* ######################################
       *  Step 6
       * Extract memory of addressed SYSTEM
       ########################################*/
      if (err=
	  vtApiDumpDram(		&galaxyConfig,	/* galaxyConfigT galaxyConfig */
					0, 			/* int system 		*/
					memDumpStatic,		/* int * dump		*/
					host,
					target
      					))
	exit (FAILURE);
      printf ("vtApiDumpDram completed\n");
      // Test le1func_context0 for LEN=0x100
      int i;
      //rdOneDramLocation (galaxyConfigT *galaxyConfig, int system ,  int daddr , int * data, targetE target)
      //      	for (i=0x20;i< 0x200;i++) printf ("%x  ,",memDumpStatic[i]);
      //for (i=0x0;i< 0x200;i+=4) printf ("%x  ,",rdOneDramLocation(galaxyConfig , 0 , i  , target));



      /* ######################################
       * Step 8
       * At this point, the application can
       * execute on the host system (X86/MB).
       * The output arrays can be compared
       * against the memDump produced
       * by the VThreads HC
       ########################################*/

      printf ("Generic driver  completed\n");

    }

  }
}

#ifdef SCTB


#endif
