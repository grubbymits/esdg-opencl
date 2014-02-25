/*******************************************
 * driver.c				 ***
 * Code example to use the VT API	 ***
 * (C) 2010 V. Chouliaras 		 ***
 * *****************************************
 ***   Proprietary and Confidential      ***
 *** vassilios@vassilios-chouliaras.com  ***
 *******************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <float.h>
#include "vtapi.h"
//#include "src/global.h"
//#include "src/platform.h"
//#include "xil_config.h"

#ifdef SCTB
#include <systemc.h>
#include "sctb.h"
SC_MODULE_EXPORT(sctb);
#endif


#define HOST X86
#define TARGET RTL


globalTargetT globalTarget;
//#define printf xil_printf
//BASICMATH_CLEAN
#define xil_printf printf
#include "iram.h"
#include "dram.h"


char * usage="driver -target [SIMSTUB SILICON]";

currentT current; 
//static int ahc; // This is the iteration variable over the active HCs
// This global makes the SCTB IF visible in non-SCTB mode


#ifdef SCTB
dbgIfT dbgIfG;

//
void driver2(dbgIfT *dbgIf)
#else
  void driver()
#endif

{
	int ahc; // This is the iteration variable over the active HCs
  //printmsg();
#ifdef SCTB
#if VTAPIDEBUG
  cout << "in driver2.c\n";
#endif
  //dbgIfG=*dbgIf; //**** ASSIGN GLOBAL FROM ARG ****//
  //globalTarget = {/*host*/X86 , /*target1*/INSIZZLE, /*target2*/RTL};
  //globalTarget.host=X86;globalTarget.target1=INSIZZLE;globalTarget.target2=RTL;
#else
  printf ("in driver2\n");
#endif

  //hostE host = MB;
  //targetE target=SILICON;

  int err;	// error code
  /* Cant specify multiple targets like SIMSTUB | INSIZLE | SILICON
     target = target | SIMSTUB; Instead, use multiple initializers */
  /* Ptr to array holding the memory dump after execution */
  unsigned int * memDump;
  unsigned int memDumpStatic[128 * 1024];

  // temp var holding the DRAM memory size (in KB)
  unsigned int dramSize;
  /* Ptr to architected state */
  oneArchStateT * oneArchStateDump;
  oneArchStateT  oneArchStateDumpStatic;

  /* ######################################
   * Step_0 : Create and initialize the galaxyConfig structure. This is used to hold all configuration
   * and state information regarding the target processor.
   * There are 2 instances of that struct; 1,2, one per target. Finally, a common galaxyConfig is declared
   * used to drive the flow downstream
   ######################################## */
  galaxyConfigT galaxyConfig1, galaxyConfig2, galaxyConfig;

  // Declare stats
  statsT insizzleStats;

#ifndef SCTB
  xil_printf ("generic driver.....\n");
#endif


#ifdef SCTB
  globalTarget.host=X86;globalTarget.target1=RTL;globalTarget.target2=NOTARGET;
#endif 

#ifdef LEON3
  printf ("Running on Leon3 host system\n");
  globalTarget.host=Leon3;globalTarget.target1=SILICON;globalTarget.target2=NOTARGET;
#endif

  /* Do the DBGIF test */
  vtApiDbgIfDiags(globalTarget.target1);

  /* Do DMA Test */
  vtApiDmaTest(globalTarget , 0x100 /* no of bytes in DMA tests */);
  //exit(0);

  // Init galaxyCongig1/2 and bail out if mismatch
  if (err=vtApiInit( & galaxyConfig1 , globalTarget.host, globalTarget.target1)) exit (err);
  if (err=vtApiInit( & galaxyConfig2 , globalTarget.host, globalTarget.target1)) exit (err);

  printf ("vtApiInit done\n");
  // At this point we are ready to perform the system tests for each target
  //if (err=vtApiSysTest( & galaxyConfig1 , globalTarget.host, globalTarget.target1)) exit (err);


  // This checks the two recovered configs - they should be identical
  if (err=vtApiChkGalaxyConfig (globalTarget, galaxyConfig1, galaxyConfig2)) exit (err);
  printf ("vtApiChkGalaxyConfig done\n");

  /* Define the common galaxyConfig from target1 (could also be target2). This is needed as in certain
     modes (X86_INSIZZLE_RTL) we need 2 galaxyCongigs, one per target.
  */
  galaxyConfig = galaxyConfig1;

  /* ######################################
   * Step_1a :
   * Allocate the memory for the SYSTEM
   * memory dump. This is based on the galaxyConfig.ctrlState.DRAM_SHARED_CONFIG0[0]. If this is not
   * populated properly, then the malloc will fail
   ######################################## */
  dramSize= (((galaxyConfig.ctrlState.DRAM_SHARED_CONFIG0[0]) >> DRAM_SHARED_CONFIG0_REG_DRAM_SIZE_SHIFT) & DRAM_SHARED_CONFIG0_REG_DRAM_SIZE_MASK);
  // FIXME: This is needed as I need galaxyConfig initialized from InSizze before I can malloc
  dramSize=128; // 0.5MB - Remember that we read the value in KB from galaxyConfig

#ifndef SCTB
  printf ("dramSize=%dK\n",dramSize);
#endif

  /* allocate memDump area and bail out if not enough mem */
  memDump=	(unsigned int*) malloc (sizeof(char) *  dramSize*1024); // malloc proper
  if ((memDump == 0)) {

#ifndef SCTB
    printf ("Unable to allocate dramSize (memDump=%x)\n",memDump);
#else
    ;
#endif
    //exit (MEMDUMPALLOCATEERROR);
  }
#ifndef SCTB
  printf ("memDump initialized. Allocated %d KBs of memory\n",dramSize);
#else
  ;
#endif

  /* ######################################
   * Step_1b :
   * Allocate the memory for the ARCHSTATE
   * dump
   ######################################## */
  if ((oneArchStateDump=(oneArchStateT*) malloc ((sizeof(oneArchStateT))))==0) {
#ifndef SCTB
    printf ("Unable to allocate oneArchStateDump (oneArchStateDump=%x)\n",oneArchStateDump);
#else
    ;
#endif
    //exit (FAILURE);
  }
#ifndef SCTB

  printf ("oneArchStateDump initialized\n");
#endif


  /* At this point, we need to determine the simulation mode. These are:
   * X86_RTL: Simple simulation mode
   * X86_INSIZZLE: Application development mode
   * X86_INSIZZLE_RTL: Co-sim mode
   * MB_SILICON: Silicon mode
   */
  //goto skipMe;
  switch (globalTarget.host)
    {
      /* X86 HOST */
    case X86:
      /* For an X86 host, we can have:
       * TARGET1=INSIZZLE, TARGET2=NOTARGET: Application development mode
       * TARGET1=INSIZZLE, TARGET2=RTL: Co-sim mode
       * TARGET1=RTL, TARGET2=NONE: Co-sim mode (no tracing)
       */

      if ((globalTarget.target1==INSIZZLE) && (globalTarget.target2==RTL)) {
	printf ("X86_INSIZZLE_RTL mode\n");
      }
      else if ((globalTarget.target1==RTL) && (globalTarget.target2==NOTARGET)) {
	printf ("X86_RTL mode\n");
      }
      else if ((globalTarget.target1==INSIZZLE) && (globalTarget.target2==NOTARGET)) {
	// TARGET1=INSIZZLE, TARGET2=NONE: Application development mode



	/* ######################################
	 * Step_1a :
	 * Use vtApiForkMultiCHC0
	 ######################################## */
	if (err=
	    vtApiForkMultiCHC0(	&galaxyConfig,  /* galaxyConfigT galaxyConfig */
				0, 				/*int system 	*/
				1, 				/*int contexts 	*/
				0,				/* int contextOffset */
				0x0,			/* pc */
				0x4000	,		/* spOffset */
				//cpuidtest_i,CPUIDTEST_INST_SIZE, cpuidtest_d, CPUIDTEST_DATA_SIZE ,
				//BASICMATH_CLEAN_i , BASICMATH_CLEAN_INST_SIZE,	BASICMATH_CLEAN_d ,	BASICMATH_CLEAN_DATA_SIZE ,
				VORONOI_i , VORONOI_INST_SIZE,	VORONOI_d ,	VORONOI_DATA_SIZE ,

				globalTarget.host,			/* hostE host	*/
				globalTarget.target1 			/*targetE target*/
				)
	    )
	  exit (FAILURE);
#if VTAPIDEBUG
	printf ("vtApiForkMultiCHC0 succesfull\n");
#endif
	/* ######################################
	 * Step_1a :
	 * Use vtApiJoinMultiCHC0
	 ######################################## */
	if (err=
	    vtApiJoinMultiCHC0(	&galaxyConfig,	/* galaxyConfigT galaxyConfig */
				0, 				/*int system 	*/
				1, 				/*int contexts 	*/
				0 	,			/* contextOffset */
				globalTarget.host,			/* hostE host	*/
				globalTarget.target1 			/*targetE target*/))
	  exit (FAILURE);
#if VTAPIDEBUG
	printf ("vtApiJoinMultiCHC0 succesfull\n");
#endif
	/* At this point, we must extract execution statistics from Insizzle. I have defined a struct in vtapi.h which
	 * must be populated by Insizzle
	 */
	if (err= vtApiextractStats(&insizzleStats, &galaxyConfig1 , globalTarget.host, globalTarget.target1)) exit (err);

	// Having extracted the stats, we can now print them on the terminal
	vtApiPrintStats( &galaxyConfig1, &insizzleStats, globalTarget.target1);
	//galaxyConfig->ctrlState.GALAXY_CONFIG & GALAXY_CONFIG_REG_SYSTEMS_MASK , /* systems from galaxyConfig1 */
	//1 /* activeContexts */, 1 /*activeHyperContexts*/);

	//goto skipvtApiDumpOneArchState;

	/* ######################################
	 *  Step 6
	 * Extract memory of addressed SYSTEM
	 ########################################*/
	if (err=
	    vtApiDumpDram(		&galaxyConfig,	/* galaxyConfigT galaxyConfig */
					0, 			/* int system 		*/
					memDump,		/* int * dump		*/
					globalTarget.host,
					globalTarget.target1))
	  exit (FAILURE);
#ifndef SCTB
	printf ("vtApiDumpDram completed\n");
#endif
	exit (SUCCESS);
      }

      else  {
	printf ("INVALID Simulation mode (X86 host)\n");
	exit (INVALIDHOSTMODE);
      }

      break;

    case MB:
      break;

    default:
      exit (INVALIDHOST);
    }
 skipMe:

#define USE_INSTRUMENTATION 0
  // Setup the counters per active HC
  for (ahc = 0 ; ahc < 1 ; ahc++)
    {
#if USE_INSTRUMENTATION
      /* ######################################
       * Step_1 :
       * Setup counter0
       ######################################## */
      if (err=
	  vtApiSetUpInstrumentation(
				    &galaxyConfig,	/* galaxyConfigT galaxyConfig */
				    0 ,				/* instrPeriphId */
				    0, 				/* int system 	*/
				    ahc, 				/* int context 	*/
				    0,				/* int hContext */
				    ahc * 16 + 0,				/* int counter */
				    HC_COMMITED_INSTRUCTIONS, /* event to be monitored */
				    globalTarget.host,			/* hostE host	*/
				    globalTarget.target1 			/*targetE target*/))
	exit (FAILURE);

      /* ######################################
       * Step_1 :
       * Setup counter1
       ######################################## */
      if (err=
	  vtApiSetUpInstrumentation(
				    &galaxyConfig,	/* galaxyConfigT galaxyConfig */
				    0 ,				/* instrPeriphId */
				    0, 				/* int system 	*/
				    ahc, 				/* int context 	*/
				    0,				/* int hContext */
				    ahc * 16 + 1,				/* int counter */
				    HC_STATE_RUNNING_CLOCKS, /* event to be monitored */
				    globalTarget.host,			/* hostE host	*/
				    globalTarget.target1 			/*targetE target*/))
	exit (FAILURE);

      /* ######################################
       * Step_1 :
       * Setup counter2
       ######################################## */
      if (err=
	  vtApiSetUpInstrumentation(
				    &galaxyConfig,	/* galaxyConfigT galaxyConfig */
				    0 ,				/* instrPeriphId */
				    0, 				/* int system 	*/
				    ahc, 				/* int context 	*/
				    0,				/* int hContext */
				    ahc * 16 + 2,				/* int counter */
				    HC_PIPE_RESTART_BRANCH, /* event to be monitored */
				    globalTarget.host,			/* hostE host	*/
				    globalTarget.target1 			/*targetE target*/))
	exit (FAILURE);

      /* ######################################
       * Step_1 :
       * Setup counter3
       ######################################## */
      if (err=
	  vtApiSetUpInstrumentation(
				    &galaxyConfig,	/* galaxyConfigT galaxyConfig */
				    0 ,				/* instrPeriphId */
				    0, 				/* int system 	*/
				    ahc, 				/* int context 	*/
				    0,				/* int hContext */
				    ahc * 16 + 3,				/* int counter */
				    HC_PIPE_RESTART_CALL, /* event to be monitored */
				    globalTarget.host,			/* hostE host	*/
				    globalTarget.target1 			/*targetE target*/))
	exit (FAILURE);

      /* ######################################
       * Step_1 :
       * Setup counter4
       ######################################## */
      if (err=
	  vtApiSetUpInstrumentation(
				    &galaxyConfig,	/* galaxyConfigT galaxyConfig */
				    0 ,				/* instrPeriphId */
				    0, 				/* int system 	*/
				    ahc, 				/* int context 	*/
				    0,				/* int hContext */
				    ahc * 16 + 4,				/* int counter */
				    HC_PIPE_RESTART_RET, /* event to be monitored */
				    globalTarget.host,			/* hostE host	*/
				    globalTarget.target1 			/*targetE target*/))
	exit (FAILURE);

      /* ######################################
       * Step_1 :
       * Setup counter5
       ######################################## */
      if (err=
	  vtApiSetUpInstrumentation(
				    &galaxyConfig,	/* galaxyConfigT galaxyConfig */
				    0 ,				/* instrPeriphId */
				    0, 				/* int system 	*/
				    ahc, 				/* int context 	*/
				    0,				/* int hContext */
				    ahc * 16 + 5,				/* int counter */
				    HC_IFEI_STALL, /* event to be monitored */
				    globalTarget.host,			/* hostE host	*/
				    globalTarget.target1 			/*targetE target*/))
	exit (FAILURE);

      /* ######################################
       * Step_1 :
       * Setup counter6
       ######################################## */
      if (err=
	  vtApiSetUpInstrumentation(
				    &galaxyConfig,	/* galaxyConfigT galaxyConfig */
				    0 ,				/* instrPeriphId */
				    0, 				/* int system 	*/
				    ahc, 				/* int context 	*/
				    0,				/* int hContext */
				    ahc * 16 + 6,				/* int counter */
				    HC_IFE_NO_FETCH, /* event to be monitored */
				    globalTarget.host,			/* hostE host	*/
				    globalTarget.target1 			/*targetE target*/))
	exit (FAILURE);

      /* ######################################
       * Step_1 :
       * Setup counter7
       ######################################## */
      if (err=
	  vtApiSetUpInstrumentation(
				    &galaxyConfig,	/* galaxyConfigT galaxyConfig */
				    0 ,				/* instrPeriphId */
				    0, 				/* int system 	*/
				    ahc, 				/* int context 	*/
				    0,				/* int hContext */
				    ahc * 16 + 7,				/* int counter */
				    HC_LSU_HOLD, /* event to be monitored */
				    globalTarget.host,			/* hostE host	*/
				    globalTarget.target1 			/*targetE target*/))
	exit (FAILURE);

      /* ######################################
       * Step_1 :
       * Setup counter8
       ######################################## */
      if (err=
	  vtApiSetUpInstrumentation(
				    &galaxyConfig,	/* galaxyConfigT galaxyConfig */
				    0 ,				/* instrPeriphId */
				    0, 				/* int system 	*/
				    ahc, 				/* int context 	*/
				    0,				/* int hContext */
				    ahc * 16 + 8,				/* int counter */
				    HC_SCRBD_S_GPR_STALL, /* event to be monitored */
				    globalTarget.host,			/* hostE host	*/
				    globalTarget.target1 			/*targetE target*/))
	exit (FAILURE);

      /* ######################################
       * Step_1 :
       * Setup counter9
       ######################################## */
      if (err=
	  vtApiSetUpInstrumentation(
				    &galaxyConfig,	/* galaxyConfigT galaxyConfig */
				    0 ,				/* instrPeriphId */
				    0, 				/* int system 	*/
				    ahc, 				/* int context 	*/
				    0,				/* int hContext */
				    ahc * 16 + 9,				/* int counter */
				    HC_SCRBD_S_BR_STALL, /* event to be monitored */
				    globalTarget.host,			/* hostE host	*/
				    globalTarget.target1 			/*targetE target*/))
	exit (FAILURE);

      /* ######################################
       * Step_1 :
       * Setup counter10
       ######################################## */
      if (err=
	  vtApiSetUpInstrumentation(
				    &galaxyConfig,	/* galaxyConfigT galaxyConfig */
				    0 ,				/* instrPeriphId */
				    0, 				/* int system 	*/
				    ahc, 				/* int context 	*/
				    0,				/* int hContext */
				    ahc * 16 + 10,				/* int counter */
				    HC_SCRBD_S_LR_STALL, /* event to be monitored */
				    globalTarget.host,			/* hostE host	*/
				    globalTarget.target1 			/*targetE target*/))
	exit (FAILURE);

      /* ######################################
       * Step_1 :
       * Setup counter11
       ######################################## */
      if (err=
	  vtApiSetUpInstrumentation(
				    &galaxyConfig,	/* galaxyConfigT galaxyConfig */
				    0 ,				/* instrPeriphId */
				    0, 				/* int system 	*/
				    ahc, 				/* int context 	*/
				    0,				/* int hContext */
				    ahc * 16 + 11,				/* int counter */
				    HC_SCRBD_STALL, /* event to be monitored */
				    globalTarget.host,			/* hostE host	*/
				    globalTarget.target1 			/*targetE target*/))
	exit (FAILURE);   		   			   				     	/* ######################################
													 * Step_1 :
													 * Setup counter12
													 ######################################## */
      if (err=
	  vtApiSetUpInstrumentation(
				    &galaxyConfig,	/* galaxyConfigT galaxyConfig */
				    0 ,				/* instrPeriphId */
				    0, 				/* int system 	*/
				    ahc, 				/* int context 	*/
				    0,				/* int hContext */
				    ahc * 16 + 12,				/* int counter */
				    C_CLOCKS, /* event to be monitored */
				    globalTarget.host,			/* hostE host	*/
				    globalTarget.target1 			/*targetE target*/))
	exit (FAILURE);

      /* ######################################
       * Step_1 :
       * Setup counter13
       ######################################## */
      if (err=
	  vtApiSetUpInstrumentation(
				    &galaxyConfig,	/* galaxyConfigT galaxyConfig */
				    0 ,				/* instrPeriphId */
				    0, 				/* int system 	*/
				    ahc, 				/* int context 	*/
				    0,				/* int hContext */
				    ahc * 16 + 13,				/* int counter */
				    HC_SYSTEM_STATE_RUNNING_OFFSET, /* event to be monitored */
				    globalTarget.host,			/* hostE host	*/
				    globalTarget.target1 			/*targetE target*/))
	exit (FAILURE);
#endif
    }
#if USE_INSTRUMENTATION
  /* ######################################
   * Step_1 :
   * Setup counter 15 to  monitor the system
   ######################################## */
  if (err=
      vtApiSetUpInstrumentation(
				&galaxyConfig,	/* galaxyConfigT galaxyConfig */
				0 ,				/* instrPeriphId */
				0, 				/* int system 	*/
				0, 				/* int context 	*/
				0,				/* int hContext */
				15,				/* int counter */
				SYSTEM_STATE_RUNNING_CLOCKS , /* event to be monitored */
				globalTarget.host,			/* hostE host	*/
				globalTarget.target1 			/*targetE target*/))
    exit (FAILURE);
#endif
  /* ######################################
   * Step_1a :
   * Use vtApiForkMultiCHC0
   ######################################## */
  if (err=
      vtApiForkMultiCHC0
      (	&galaxyConfig,	/* galaxyConfigT galaxyConfig */
	0, 				/*int system 	*/
	1, 				/*int contexts 	*/
	0, // contextOffset
	0x0,			/* pc */
	0x4000	,		/* spOffset */
	VORONOI_i , VORONOI_INST_SIZE,	VORONOI_d ,	VORONOI_DATA_SIZE ,
	globalTarget.host, 			/*targetE target*/
	globalTarget.target1 			/*targetE target*/
	)
      )
    exit (FAILURE);
#if VTAPIDEBUG
  printf ("vtApiForkMultiCHC0 succesfull\n");
#endif
  /* ######################################
   * Step_1a :
   * Use vtApiJoinMultiCHC0
   ######################################## */
  if (err=
      vtApiJoinMultiCHC0(	&galaxyConfig,	/* galaxyConfigT galaxyConfig */
				0, 				/*int system 	*/
				1, 				/*int contexts 	*/
				0 	,			/* contextOffset */
				globalTarget.host,			/* hostE host	*/
				globalTarget.target1 			/*targetE target*/))
    exit (FAILURE);
#if VTAPIDEBUG
  printf ("vtApiJoinMultiCHC0 succesfull\n");
#endif

	/* At this point, we must extract execution statistics from Insizzle. I have defined a struct in vtapi.h which
	 * must be populated by Insizzle
	 */
	if (err= vtApiextractStats(&insizzleStats, &galaxyConfig1 , globalTarget.host, globalTarget.target1)) exit (err);

	// Having extracted the stats, we can now print them on the terminal
	vtApiPrintStats( &galaxyConfig1, &insizzleStats, globalTarget.target1);

  goto skipvtApiDumpOneArchState;
  /* ######################################
   *  Step 7
   * Extract architected space of addressed
   * SYSTEM.CONTEXT.HYPERCONTEXT
   ########################################*/

  if (err=
      vtApiDumpOneArchState(	&galaxyConfig,	/* galaxyConfigT galaxyConfig */
				0, 		/* int system 		*/
				0,		/* int context		*/
				0,		/* int hypercontext */
				oneArchStateDump, /* oneArchStateT * dump */
				globalTarget.host,
				globalTarget.target1	/* targetE target	*/
				))
    exit (FAILURE);
#ifndef SCTB
  printf ("vtApiDumpArchState completed\n");
#endif
 skipvtApiDumpOneArchState:

  // vtApiextractStats(statsT* stats, galaxyConfigT *galaxyConfig, hostE host , targetE target);



#ifndef SCTB
  printf ("Generic driver  completed\n");
#endif

  exit(0);
}


#ifdef SCTB
void crap(void) {
  cout <<"crap\n";
}
#endif

#ifdef SCTB
void driver1(	dbgIfT * dbgIf	)
{
  cout << "in driver1.c\n";
  dbgIf->vc_addr->write("0xcafef00d");
  //vc_rd->write(SC_LOGIC_1);
}
#endif
#ifdef MB_SILICON
int main(void)
//int mbMain(void)
#else
int main1(void)
#endif
{
  // Supported modes  // X86_INSIZZLE = Straight insizzle sim
  // X86_INSIZZLE_RTL = Insizzle + RTL co-sim
  printf("Ready...\n");
#ifdef X86_INSIZZLE
  printf ("X86_INSIZZLE\n");
  globalTarget.host=X86;globalTarget.target1=INSIZZLE;globalTarget.target2=NOTARGET;
#endif
#ifdef SCTB
  printf ("X86_RTL\n");
  globalTarget.host=X86;globalTarget.target1=RTL;globalTarget.target2=NOTARGET;
  driver2(&dbgIfG);
  
#endif
#ifdef MB_SILICON
  globalTarget.host=MB;globalTarget.target1=SILICON;
  driver();

#endif
}
