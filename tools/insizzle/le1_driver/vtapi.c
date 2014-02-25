/************************************************************************************************
  * vtapi.c
 * VT low-level interface API
 * (C) 2010 V. Chouliaras
 * Proprietary and Confidential
 * vassilios@vassilios-chouliaras.com
 ************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#ifdef SCTB
#include <systemc.h>
#endif

#include "vtapi.h"
//#include "stubs.h"
//define printf xil_printf
//#include "xil_config.h"
//#include "sim_stub.h"

#ifdef SCTB
extern dbgIfT dbgIfG;
#include "stubs.h"
#endif

char * errMsg[32] ;
extern globalTargetT globalTarget;
/************************************************************************************************
 * Print error message MACRO
 ************************************************************************************************/
#define ERROR (errCode) {extern int errCode ; extern char * errMsg ; init_errors() ; printf (errMsg[errCode] , errCode)}
//#define SWAPBYTE (data) {unsigned int temp; unsigned char * wcptr=&temp; *wcptr=instructions[i+3];*(wcptr+1)=instructions[i+2];*(wcptr+2)=instructions[i+1];*(wcptr+3)=instructions[i+0];


/************************************************************************************************
 * This checks the two galaxyConfig(1/2) recovered from the two targets.
They should be identical; Otherwise,
* it reports that
************************************************************************************************/
int vtApiChkGalaxyConfig(globalTargetT globalTarget, galaxyConfigT galaxyConfig1,  galaxyConfigT  galaxyConfig2)
{
  printf ("in vtApiChkGalaxyConfig\n");
  return (SUCCESS);
}
unsigned int swapByte(unsigned int a)
{
  unsigned int t;
  unsigned char wcptr[4];//=&t;
  unsigned int i, src , dst=0;
  unsigned char * sPtr, *dPtr;
  sPtr = (unsigned char *) & src;
  dPtr = (unsigned char *) & dst;
  src=a;
  for (i=0;i<4;i++) *(dPtr + i) = * (sPtr + 3 -i);

  /*
   *wcptr=a>>24 & 0xff;
   *(wcptr+1)=a>>16 & 0xff;
   *(wcptr+2)=a>>8 & 0xff;
   *(wcptr+3)=a>>0 & 0xff;
   t = (unsigned int)((a>>24) & 0xff) |
   (unsigned int)((a>>16) & 0xff) |
   (unsigned int)((a>>8) & 0xff) |
   (unsigned int)((a>>0) & 0xff);
  */
#if VTAPIDEBUG
  printf ("SWAPBYTE: In (%x) Out (%x)\n",src,dst);
#endif
  return(dst);
}

// Low-level functions
/************************************************************************************************
 *	vtDbgRd performs a low-level debug port read operations. Can be issued by the HOST (-DHOST)
 * or the FLI (-DHOST -DFLI
 ************************************************************************************************/
//#ifndef SCTB
int vtDbgRd (unsigned int addr , unsigned int * rdata, targetE target)
{

  unsigned int data=0;
#ifdef SCTB
  char lstring[10];
#if VTAPIDEBUG
  printf ("in vtDbgRd (addr=%x)\n",addr);
#endif
  //wait(10, SC_NS);
  dbgIfG.vc_din->write("ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ");
  //  sprintf (lstring,"0x%x",addr);	dbgIfG.vc_addr->write(lstring);
  sprintf (lstring,"%#8.8x",(unsigned int) addr);	dbgIfG.vc_addr->write(lstring);
  dbgIfG.vc_rd->write(SC_LOGIC_1);
  wait(10, SC_NS);
  dbgIfG.vc_rd->write(SC_LOGIC_0);
  *rdata=(unsigned int)dbgIfG.vc_dout->read().to_uint();

#if VTAPIDEBUG
  printf ("vtDbgRd:Done wait: Read %x\n",*rdata);
#endif
  //data=(static_cast<int>(dbgIfG.vc_addr->read());
#else
  //printf ("vtDbgRd %x\n",addr);
#if AXI==0	/* Working with PLB which is BE=> no byte swaping */
  data = *(volatile unsigned int *)addr;
  *rdata=data;
  //data=Xil_In32BE(addr);
#else 	/* Working with AXI4 which is LE (Attention to the BYTE SWAP of the AXI->PLB bridge if one exists! */
  //data=Xil_In32LE(addr);
  ;
#endif
#endif
  /* THis function always returns success as it reads from teh debug space registers */
#ifdef SCTB  
wait(10, SC_NS);
#endif
  return (SUCCESS);

}
//#endif
/************************************************************************************************
 *	vtDbgWr performs a low-level debug port write operation. Can be issued by the HOST (-DHOST)
 * or the FLI (-DHOST -DFLI
 ************************************************************************************************/
//#ifndef SCTB
int vtDbgWr (unsigned int addr , unsigned int wrdata , targetE target)
{

#ifdef SCTB
  char lstring[10];
  //wait(10, SC_NS);
#if VTAPIDEBUG
  printf("vtDbgWr: Writing addr %x with value %x\n",addr, wrdata);
#endif
  sprintf (lstring,"%#8.8x",(unsigned int) addr);	dbgIfG.vc_addr->write(lstring);
#if VTAPIDEBUG
  printf ("lstring(addr)=%s\n",lstring);
#endif
  //dbgIfG.vc_addr->write("0x76543210");
  sprintf (lstring,"%#8.8x",(unsigned int) wrdata);dbgIfG.vc_din->write(lstring);
#if VTAPIDEBUG
  printf ("lstring(wdata)=%s\n",lstring);
#endif
  dbgIfG.vc_rd->write(SC_LOGIC_0);
  dbgIfG.vc_wr->write(SC_LOGIC_1);
  wait(10, SC_NS);
  dbgIfG.vc_wr->write(SC_LOGIC_0);
#else
#if AXI==0 /* Working with PLB which is BE=> no byte swaping */
  //printf ("*%x=%x\n",addr,wrdata);
  *(volatile int *)addr=wrdata;
#else /* Working with AXI4 which is LE (Attention to the BYTE SWAP of the AXI->PLB bridge if one exists! */
  //Xil_Out32LE(addr , wrdata);
  ;
#endif
#endif
  //printf ("VTDBGWR: *%x=%x\n",addr,wrdata);
#ifdef SCTB
  wait(10, SC_NS);
#endif
  return (SUCCESS);
}
//#endif
/************************************************************************************************
 *	Initialize the global errMsg array. FIXME
 ************************************************************************************************/
void init_errors(void)
{
  errMsg[SUCCESS]="%d: Success\n";
  errMsg[FAILURE]="%d: Failure\n";
  errMsg[ERROR_VT_CANT_ACCESS_DEBUG_PORT]="%d: VTff cant access debug port\n";
}
#ifdef HOST

#ifdef FLI

#include "mti.h"

#endif

#endif


/************************************************************************************************
 * Perform a read to a debug I/F register
 ************************************************************************************************/
int vtDbgRdIf(unsigned int reg ,unsigned int *val,targetE target )
{
  //#ifdef HOST
  unsigned int rval;
#if VT_RM_DEBUG_IF
  /* -DHOST and VT_RM_DEBUG_IF */
#else
  vtDbgRd(DBG_BASE + reg * 4, &rval , target);
  *val=rval;
  //printf("DBGIF_R_%d (addr:%x)=%x\n",reg ,DBG_BASE + reg * 4,rval);
  return (SUCCESS);
#endif
}



/************************************************************************************************
 * Busy-wait until BUSY_REG in debug I/F is 0
 ************************************************************************************************/
void vtBusyWait(targetE target)
{
  unsigned int val;

  do {
    vtDbgRdIf(BUSY_REG, &val , target);
#if VTAPIDEBUG
    printf ("vtBusyWait: Read BUSY_REG=%x\n",val);
#endif
  }
  while (val >> 2);
}


/************************************************************************************************
 *	vtDbgWr performs a low-level debug port write operation. Can be issued by the HOST (-DHOST)
 * or the FLI (-DHOST -DFLI
 ************************************************************************************************/
//#ifndef SCTB
int vtDbgWrIf (unsigned int reg , unsigned int wrdata , targetE target)
{
  //unsigned int rdback;
#if AXI==0 /* Working with PLB which is BE=> no byte swaping */
#if VTAPIDEBUG
  printf ("vtDbgWrIf: Writing reg %d (addr %x) with %x\n",reg , DBG_BASE + reg * 4 , wrdata);
#endif
  vtDbgWr(DBG_BASE + reg * 4 , wrdata , target);

#else /* Working with AXI4 which is LE (Attention to the BYTE SWAP of the AXI->PLB bridge if one exists! */
  //Xil_Out32LE(DBG_BASE + reg * 4 , wrdata);
  ;
#endif
  //vtDbgRdIf(DBG_BASE + reg * 4, &rdback , target);
  //vtDbgRd(DBG_BASE + reg * 4, &rdback , target);
#if VTAPIDEBUG
  //printf ("VTDBGWRDIF: Writing DBGIF_R%d with %x. Read back %x\n",reg,wrdata,rdback);
#endif
  return (SUCCESS);
}

/************************************************************************************************
 * Set the current [system.context.hypercontext.cluster]
 ************************************************************************************************/
int vtDbgsetCurrent(unsigned int system , unsigned int context ,unsigned int hypercontext , unsigned int cluster , hostE host , targetE target)
{
  //printf("setCurrent S[%d]C[%d]HC[%d]CL[%d] - Target %d\n",current.sys , current.c , current.hc, current.cl , target);
#if VTAPIDEBUG
  printf ("vtDbgsetCurrent:Host=%d\tTarget=%d\n",host,target);
#endif

  int err=FAILURE;
  if (target==NOTARGET) {
    return(FAILURE);
  }
  else if (target==SIMSTUB) {
    return(SUCCESS);
  }
  else if (target==INSIZZLE) {
    return (insizzleSetCurrent(system, context, hypercontext, cluster));
  }
  else if (target == SILICON) {
    if (host == X86) {
      printf ("HOST->SILICON path not yet implemented\n");
    }
    else if (host == MB) {
      if (VT_RM_DEBUG_IF) {

      }
      else /* Legacy I/F */ {
	err=vtDbgWrIf(SYSTEM_REG , system , target);
	  printf ("vtDbgsetCurrent:Inner1 done\n");
	err=err | vtDbgWrIf(CONTEXT_REG , context, target);
	  printf ("vtDbgsetCurrent:Inner2 done\n");
	err=err | vtDbgWrIf(HYPERCONTEXT_REG , hypercontext, target);
	  printf ("vtDbgsetCurrent:Inner3 done\n");
	err=err | vtDbgWrIf(CLUSTER_REG , cluster, target);
	  printf ("vtDbgsetCurrent:Inner4 done\n");
#if VTAPIDEBUG
	{
	  int localsys , localcont , localhc;
	  //err=vtDbgWrIf(localsys , system , target);
	  //printf ("vtDbgsetCurrent: S=%d\tC=%d\tHC=%d\n",localsys , localcont , localhc);
	  //printf ("vtDbgsetCurrent: S=%d\tC=%d\tHC=%d\n",system , context , hypercontext);

	}
#endif
      }
    }
  }
  else
    return (FAILURE);
}


/************************************************************************************************
 * Perform a read control register operation via the debug interface
 * for register reg. Return read value on val; Return error code
 ************************************************************************************************/
int vtDbgRdCtrl(hostE host, targetE target , unsigned int reg , unsigned int *val)
{
  //#ifdef HOST
  unsigned int rval;
#if VT_RM_DEBUG_IF
  /* -DHOST and VT_RM_DEBUG_IF */
#else
  /* -DHOST and LEGACY I/F */
  vtDbgWrIf(ADDR_REG, CR_DBG_BASE + reg  , target);
  vtDbgWrIf(CMD_REG, DBGCMD_LEGACY_RD, target);
  vtBusyWait(target);
  vtDbgRdIf(RDDATA1_REG, &rval , target);
#if VTAPIDEBUG
  //printf ("vtDbgRdCtrl: CTRL_R[%d]=%x\n",reg , rval);
#endif
  *val=rval;
  return (SUCCESS);
#endif
}
/************************************************************************************************
 * Perform a read control register operation via the debug interface
 * for register reg. Return read value on val; Return error code
 ************************************************************************************************/
int vtDbgWrCtrl(hostE host, targetE target , unsigned int reg , unsigned int val)
{
  //#ifdef HOST
  unsigned int rval;
#if VT_RM_DEBUG_IF
  /* -DHOST and VT_RM_DEBUG_IF */
#else
  /* -DHOST and LEGACY I/F */
  vtDbgWrIf(ADDR_REG, CR_DBG_BASE + reg * 4  , target); // DANGER - FIXME!!!!
  vtDbgWrIf(WRDATA1_REG, val, target);
  vtDbgWrIf(CMD_REG, DBGCMD_LEGACY_WR, target);
  vtBusyWait(target);
#if VTAPIDEBUG
  printf ("vtDbgWrCtrl: R[%d]=%x\n",reg,val);
#endif
  return (SUCCESS);
#endif
}
/*#ifdef FLI
  #if VT_RM_DEBUG_IF
  -DHOST -DFLI and VT_RM_DEBUG_IF
  #else
  -DHOST -DFLI and LEGACY I/F
  #endif

  #else
  // Host op - NOT FLI
  #endif

  #else
  // VT op
  //ERROR (ERROR_VT_CANT_ACCESS_DEBUG_PORT)
  return(ERROR_VT_CANT_ACCESS_DEBUG_PORT);
  #endif	*/


/************************************************************************************************
 *	dbgWrDebug performs a low-level debug port write operation. Issued only by the HOST (-DHOST)
 * or the FLI (-DHOST -DFLI)
 ************************************************************************************************/
int dbgWrDebug (unsigned int addr , unsigned int wrdata)
{

}
/************************************************************************************************
 *	
 * or the FLI (-DHOST -DFLI)
 ************************************************************************************************/
int chkMem  (char * buf1 , char * buf2 , int bytes )
{
  int i;
  for (i = 0 ; i < bytes ; i++)
    {
      if (!(*buf1++ == *buf2++))
	{
	  printf ("Failure at index %d\n",i);
	  return (FAILURE);
	}
    }
  return (SUCCESS);
}

/************************************************************************************************
 *	
 * or the FLI (-DHOST -DFLI)
 ************************************************************************************************/
int vtApiDmaTest  (globalTargetT globalTarget , int len /*bytes*/)
{
  int i; // The number of beats in terms of size of the memory port
  int * buf1 , * buf2;
#if DEBUG
  printf ("vtApiDmaTests \n");
#endif 
  switch (globalTarget.host)
    {
    case LEON3 :
      printf ("Ready to run DmaTest for Leon3\n");
      buf1= (int*) (malloc(len * sizeof(int)));
      //buf1 = (int*)malloc(len * sizeof(int));
      if (!buf1)
	{
	  printf ("Unable to malloc buf1\n");
	  return (FAILURE);
	}
      buf2= (int*) (malloc(len * sizeof(int)));
      //if ((buf2=(malloc(len * sizeof(int)))) == 0); 
      if (!buf2)
	{
	  printf ("Unable to malloc buf2\n");
	  return (FAILURE);
	}
      // Now, call vtApiDma
      vtApiDma(buf1 /* src */ , buf2 /* dest */ , len /* DMA len in bytes */ , globalTarget);
      // Now, check the two buffers
      if (chkMem((char*)buf1 , (char*)buf2 , len /* bytes */) == SUCCESS)
	printf ("DMA is good\n");
      else
	printf ("DMA Broken\n");

      break;
      
    default :
      printf ("vtApiDmaTests: Unknown host\n");
    }
  return (SUCCESS);
}



/************************************************************************************************
 *	vt
 * or the FLI (-DHOST -DFLI)
 ************************************************************************************************/
int vtApiDma  (int * buf1 , int *  buf2  , unsigned int bsize ,globalTargetT globalTarget )
{
  int i; // The number of beats in terms of size of the memory port
  //#if DEBUG
  printf ("vtApiDmaIn : Transferring %x bytes from address %x to address %x\n",bsize, buf1, buf2);
  //#endif
  int ctrl = 0 ;
  
  switch (globalTarget.host)

    {
#define AHBDMA_BROKEN


    case LEON3 :
      /* 
	 0x0(31:0) = srcaddr
	 0x4(31:0) = dstaddr
	 0x8(20:0) = enable (20) & srcinc (19:18) & dstinc (17:16) & len (15:0)
	 >> Think that srcinc and dstinc reflect the AHB size (0x0=byte, 0x1=hw, 0x2=W)
	 NB: AHBDMA_APB is passed by the Makefile as -DAHBDMA_APB=13
       */
#ifdef AHBDMA_BROKEN 
  for (i = 0 ; i < bsize ; i++) *buf2++ == *buf1++;
#else
      // Setup src
       * (volatile int *) (0x80000000 + AHBDMA_APB * 0x100 + 0x0) = (volatile int *) buf1;
       printf ("src=%x\n", *(volatile int *)(0x80000000 + AHBDMA_APB * 0x100 + 0x0)); 
      // Setup dest
       * (volatile int *) (0x80000000 + AHBDMA_APB * 0x100 + 0x4) = (volatile int *) buf2;
       printf ("dest=%x\n", *(volatile int *)(0x80000000 + AHBDMA_APB * 0x100 + 0x4)); 
      // Setup ctrl
      ctrl =  
	((0x1 << 20) & 0xffffffff) | 
	((0x2 << 18) & 0xffffffff) | 
	((0x2 << 16) & 0xffffffff) |
	(bsize & 0xffff);
      printf ("Ctrl word=%x\n",ctrl);
      * (volatile int *) (0x80000000 + AHBDMA_APB * 0x100 + 0x8) = (volatile int *) ctrl;
      // DMA is active!
      // Busy-wait ?
      //for (i=0;i<0xff;i++) 
      //while ((* (volatile int *) (0x80000000 + AHBDMA_APB * 0x100 + 0x8)) & 0xffff) 
	{
	  printf ("AHBDMA ctrl=%x\n", *(volatile int *)(0x80000000 + AHBDMA_APB * 0x100 + 0x8)); 	;
	}

	{	
	    //printf ("AHBDMA ctrl=%x\n", *(volatile int *)(0x80000000 + AHBDMA_APB * 0x100 + 0x8)); 
	}
#endif
      break;

    default :
      printf ("vtApiDma: Unknown host\n");

      break;

    }
  

}


/************************************************************************************************
 * Populate the hyperContextConfig of the addressed context
 ************************************************************************************************/

/*void getHyperContextConfig (hyperContextConfigT * hyperContextConfig , int hyperContext)
  {
  int rval;	 value recovered from read ops

  dbgWrDebug(HYPERCONTEXT_REG , hyperContext);
  vtDbgRdCtrl(HYPERCONTEXT_STATIC_REGFILE_CONFIG_REG  , &rval);
  //hyperContextConfig->sGprFileSize	=	(rval >> HYPERCONTEXT_STATIC_REGFILE_CONFIG_REG_S_GPR_FILE_SIZE_SHIFT)	& HYPERCONTEXT_STATIC_REGFILE_CONFIG_REG_S_GPR_FILE_SIZE_MASK;
  //hyperContextConfig->sFprFileSize 	= 	(rval >> HYPERCONTEXT_STATIC_REGFILE_CONFIG_REG_S_FPR_FILE_SIZE_SHIFT) 	& HYPERCONTEXT_STATIC_REGFILE_CONFIG_REG_S_FPR_FILE_SIZE_MASK;
  //hyperContextConfig->sVrFileSize 	= 	(rval >> HYPERCONTEXT_STATIC_REGFILE_CONFIG_REG_S_VR_FILE_SIZE_SHIFT) 	& HYPERCONTEXT_STATIC_REGFILE_CONFIG_REG_S_VR_FILE_SIZE_MASK;
  //hyperContextConfig->sPrFileSize 	= 	(rval >> HYPERCONTEXT_STATIC_REGFILE_CONFIG_REG_S_PR_FILE_SIZE_SHIFT) 	& HYPERCONTEXT_STATIC_REGFILE_CONFIG_REG_S_PR_FILE_SIZE_MASK;
  }*/

/************************************************************************************************
 * Populate the clusterConfig of the addressed context
 ************************************************************************************************/

/*void getClusterConfig (clusterConfigT * clusterConfig ,  int cluster)
  {
  int rval;	 value recovered from read ops

  vtDbgRdCtrl(CLUSTER_CONFIG_REG  , &rval);
  clusterConfig->sCorePresent = 		(rval >> CLUSTER_CONFIG_REG_SCORE_PRESENT_SHIFT) & CLUSTER_CONFIG_REG_SCORE_PRESENT_MASK;
  clusterConfig->vCorePresent = 		(rval >> CLUSTER_CONFIG_REG_VCORE_PRESENT_SHIFT) & CLUSTER_CONFIG_REG_VCORE_PRESENT_MASK;
  clusterConfig->fCorePresent = 		(rval >> CLUSTER_CONFIG_REG_FCORE_PRESENT_SHIFT) & CLUSTER_CONFIG_REG_FCORE_PRESENT_MASK;
  clusterConfig->cCorePresent = 		(rval >> CLUSTER_CONFIG_REG_CCORE_PRESENT_SHIFT) & CLUSTER_CONFIG_REG_CCORE_PRESENT_MASK;
  }*/


/************************************************************************************************
 * Populate the contextConfig of the addressed context
 ************************************************************************************************/

/*void getContextConfigDEAD (contextConfigT * contextConfig , int context)
  {
  int cluster , hyperContext;
  int rval;	 value recovered from read ops

  dbgWrDebug(CONTEXT_REG , context);

  vtDbgRdCtrl(CONTEXT_CONFIG_REG  , &rval);
  //contextConfig->clusters = 			(rval >> CONTEXT_CONFIG_REG_CLUSTERS_SHIFT) & CONTEXT_CONFIG_REG_CLUSTERS_MASK;
  contextConfig->hyperContexts = 		(rval >> CONTEXT_CONFIG_REG_HYPERCONTEXTS_SHIFT) & CONTEXT_CONFIG_REG_HYPERCONTEXTS_MASK;

  //for (cluster=0; cluster < contextConfig->clusters; cluster++)
  {
  populate contextConfig[context]
  //  getClusterConfig(& (contextConfig->clusterConfig[cluster] )  , cluster );
  }

  for (hyperContext=0; hyperContext < contextConfig->hyperContexts; hyperContext++)
  {
  populate contextConfig[context]
  getHyperContextConfig(& (contextConfig->hyperContextConfig[hyperContext] ) , hyperContext );
  }

  }
*/
/************************************************************************************************
 * Populate the systemConfig of the addressed system
 ************************************************************************************************/

/*void getSystemConfig (systemConfigT * systemConfig , int system)
  {
  int context;
  int rval;	 value recovered from read ops

  Select system
  dbgWrDebug(SYSTEM_REG , system);
  vtDbgRdCtrl(SYSTEM_CONFIG_REG  , &rval);
  systemConfig->contexts =  (rval >> SYSTEM_CONFIG_REG_CONTEXTS_SHIFT) & SYSTEM_CONFIG_REG_CONTEXTS_MASK;
  systemConfig->scalarSysPresent = 	(rval >> SYSTEM_CONFIG_REG_SCALARSYS_PRESENT_SHIFT) &  SYSTEM_CONFIG_REG_SCALARSYS_PRESENT_MASK;
  systemConfig->periphPresent = 		(rval >> SYSTEM_CONFIG_REG_PERIPH_PRESENT_SHIFT) & SYSTEM_CONFIG_REG_PERIPH_PRESENT_MASK;
  #ifndef SCTB
  systemConfig->iArch = 			(rval >> SYSTEM_CONFIG_REG_IARCH_SHIFT) & SYSTEM_CONFIG_REG_IARCH_MASK;
  #endif
  for (context=0; context <systemConfig->contexts; context++)
  {
  populate contextConfig[context]
  // getContextConfig(& (systemConfig->contextConfig[context] ) , context);
  }
  }*/

/************************************************************************************************
 * Enumerate all capabilities of the GALAXY and populate galaxyConfig with all ctrl space
 ************************************************************************************************/

int getGalaxyConfig (hostE host, targetE target , galaxyConfigT * galaxyConfig)
{
  unsigned int sys;	/* Iteration variable for CMP systems in GALAXY */
  unsigned int rval;	/* value recovered from read ops				*/
  unsigned int contexts; /* iteration variable for contexts */
  unsigned int hcontexts; /* itetation variable for hypercontexts */
  unsigned int templ; /* iteration variable for cluster_templ */
  //printf ("getGalaxyConfig:Host=%d\tTarget=%d\n",host,target);
  vtDbgsetCurrent(0,0,0,0,host,target);

  //printf ("getGalaxyConfig:Finished vtDbgsetCurrent\n");

  //===============================================
  // GALAXY_CONFIG_REG
  vtDbgRdCtrl(host , target, GALAXY_CONFIG_REG , &rval);
#if VTAPIDEBUG
  printf ("GALAXY_CONFIG_REG=%x\n",rval);
#endif
  galaxyConfig->ctrlState.GALAXY_CONFIG=rval;

  /* Iterate over all identified systems */
  //===============================================
  // SYSTEM_CONFIG_REG[S]
  for (sys=0 ; sys < galaxyConfig->ctrlState.GALAXY_CONFIG & GALAXY_CONFIG_REG_SYSTEMS_MASK; sys++) {
    vtDbgsetCurrent(sys,0,0,0,host,target);
    printf ("S[%d]\n",sys);
    //===============================================
    // SYSTEM_CONFIG_REG
    vtDbgRdCtrl(host , target, SYSTEM_CONFIG_REG , &rval);
    printf ("(%d)-SYSTEM_CONFIG_REG[%d]=%x\n",SYSTEM_CONFIG_REG,sys,rval);
    galaxyConfig->ctrlState.SYSTEM_CONFIG[sys]=rval;
    printf ("S%d has a %d CONTEXTS\n",sys,(galaxyConfig->ctrlState.SYSTEM_CONFIG[sys] & SYSTEM_CONFIG_REG_CONTEXTS_MASK));
    //===============================================
    // DRAM_SHARED_CONFIG0_REG
    vtDbgRdCtrl(host , target, DRAM_SHARED_CONFIG0_REG , &rval);
    printf ("(%d)-DRAM_SHARED_CONFIG0_REG[%d]=%x\n",DRAM_SHARED_CONFIG0_REG,sys,rval);
    galaxyConfig->ctrlState.DRAM_SHARED_CONFIG0[sys]=rval;
    for (contexts=0; contexts<(galaxyConfig->ctrlState.SYSTEM_CONFIG[sys] & SYSTEM_CONFIG_REG_CONTEXTS_MASK); contexts++) {
      printf ("S[%d]C[%d]\n",sys,contexts);
      //===============================================
      // CONTEXT_CONFIG_REG
      //setCurrent(sys,contexts,0,0,host,target);
      //vtDbgRdIf(RDDATA1_REG, &rval , target);  printf ("RDDATA1_REG BEFORE reading CONTEXT_CONFIG_REG=%x\n",rval);
      vtDbgRdCtrl(host , target, CONTEXT_CONFIG_REG , &rval);
      //vtDbgRdIf(RDDATA1_REG, &rval , target);  printf ("RDDATA1_REG AFTER reading CONTEXT_CONFIG_REG=%x\n",rval);

      printf ("(%d)-CONTEXT_CONFIG_REG[%d][%d]=%x\n",CONTEXT_CONFIG_REG,sys,contexts,rval);
      galaxyConfig->ctrlState.CONTEXT_CONFIG[sys][contexts]=rval;
      //===============================================
      // IFE_SIMPLE_IRAM_PRIV_CONFIG0_REG
      vtDbgsetCurrent(sys,contexts,0,0,host,target);
      vtDbgRdCtrl(host , target, IFE_SIMPLE_IRAM_PRIV_CONFIG0_REG , &rval);
      printf ("(%d)-IFE_SIMPLE_IRAM_PRIV_CONFIG0_REG[%d][%d]=%x\n",IFE_SIMPLE_IRAM_PRIV_CONFIG0_REG,sys,contexts,rval);
      galaxyConfig->ctrlState.IFE_SIMPLE_IRAM_PRIV_CONFIG0[sys][contexts]=rval;
      //printf ("S[%d]C[%d] has %d HCs (R=%x)\n",sys,contexts,(((galaxyConfig->ctrlState.context_config_reg[sys][contexts]) >>CONTEXT_CONFIG_REG_HCONTEXTS_SHIFT) & CONTEXT_CONFIG_REG_HCONTEXTS_MASK),rval);
      for (hcontexts=0 ; hcontexts < (((galaxyConfig->ctrlState.CONTEXT_CONFIG[sys][contexts]) >>CONTEXT_CONFIG_REG_HCONTEXTS_SHIFT) & CONTEXT_CONFIG_REG_HCONTEXTS_MASK) ; hcontexts++) {
	printf ("S[%d]C[%d]HC[%d]\n",sys,contexts,hcontexts);
	//===============================================
	// HCONTEXT_CONFIG_REG[S][C][HC]
	vtDbgsetCurrent(sys,contexts,hcontexts,0,host,target);
	vtDbgRdCtrl(host , target, HCONTEXT_CONFIG_REG , &rval);
	printf ("(%d)-HCONTEXT_CONFIG_REG[%d][%d][%d]=%x\n",HCONTEXT_CONFIG_REG,sys,contexts,hcontexts,rval);
	galaxyConfig->ctrlState.HCONTEXT_CONFIG[sys][contexts][hcontexts]=rval;
	//===============================================
	// HCONTEXT_CLUST_TEMPL0[S][C][HC]
	vtDbgsetCurrent(sys,contexts,hcontexts,0,host,target);
	vtDbgRdCtrl(host , target, HCONTEXT_CLUST_TEMPL0_REG , &rval);
	printf ("(%d)-HCONTEXT_CLUST_TEMPL0_REG[%d][%d][%d]=%x\n",HCONTEXT_CLUST_TEMPL0_REG,sys,contexts,hcontexts,rval);
	galaxyConfig->ctrlState.HCONTEXT_CLUST_TEMPL0[sys][contexts][hcontexts]=rval;
	//===============================================
	// HCONTEXT_CLUST_TEMPL1[S][C][HC]
	vtDbgsetCurrent(sys,contexts,hcontexts,0,host,target);
	vtDbgRdCtrl(host , target, HCONTEXT_CLUST_TEMPL1_REG , &rval);
	printf ("(%d)-HCONTEXT_CLUST_TEMPL1_REG[%d][%d][%d]=%x\n",HCONTEXT_CLUST_TEMPL1_REG,sys,contexts,hcontexts,rval);
	galaxyConfig->ctrlState.HCONTEXT_CLUST_TEMPL_INST1[sys][contexts][hcontexts]=rval;
	//===============================================
	// HCONTEXT_CLUST_TEMPL_INST0_REG[S][C][HC]
	vtDbgsetCurrent(sys,contexts,hcontexts,0,host,target);
	vtDbgRdCtrl(host , target, HCONTEXT_CLUST_TEMPL_INST0_REG , &rval);
	printf ("(%d)-HCONTEXT_CLUST_TEMPL_INST0_REG[%d][%d][%d]=%x\n",HCONTEXT_CLUST_TEMPL_INST0_REG,sys,contexts,hcontexts,rval);
	galaxyConfig->ctrlState.HCONTEXT_CLUST_TEMPL_INST0[sys][contexts][hcontexts]=rval;
	//===============================================
	// HCONTEXT_CLUST_TEMPL_INST1_REG[S][C][HC]
	vtDbgsetCurrent(sys,contexts,hcontexts,0,host,target);
	vtDbgRdCtrl(host , target, HCONTEXT_CLUST_TEMPL_INST1_REG , &rval);
	printf ("(%d)-HCONTEXT_CLUST_TEMPL_INST1_REG[%d][%d][%d]=%x\n",HCONTEXT_CLUST_TEMPL_INST1_REG,sys,contexts,hcontexts,rval);
	galaxyConfig->ctrlState.HCONTEXT_CLUST_TEMPL_INST1[sys][contexts][hcontexts]=rval;
      }
      goto kkk;

      for (templ=0 ; templ < ((galaxyConfig->ctrlState.CONTEXT_CONFIG[sys][contexts]) >>CONTEXT_CONFIG_REG_CLUST_TEMPL_SHIFT) & CONTEXT_CONFIG_REG_CLUST_TEMPL_MASK ; templ++) {
	//===============================================
	// CLUST_TEMPL_CONFIG_REG[S][C][CL_TEMPL]
	vtDbgsetCurrent(sys,contexts,0,0,host,target);
	vtDbgRdCtrl(host , target, CLUST_TEMPL_CONFIG_REG , &rval);
	galaxyConfig->ctrlState.CLUST_TEMPL_CONFIG[sys][contexts][templ]=rval;
	//===============================================
	// CLUST_TEMPL_STATIC_REGFILE_CONFIG_REG[S][C][CL_TEMPL]
	vtDbgsetCurrent(sys,contexts,0,0,host,target);
	vtDbgRdCtrl(host , target, CLUST_TEMPL_STATIC_REGFILE_CONFIG_REG , &rval);
	galaxyConfig->ctrlState.CLUST_TEMPL_STATIC_REGFILE_CONFIG[sys][contexts][templ]=rval;
	//===============================================
	// CLUST_TEMPL_SCORE_CONFIG_REG[S][C][CL_TEMPL]
	vtDbgsetCurrent	(sys,contexts,0,0,host,target);
	vtDbgRdCtrl(host , target, CLUST_TEMPL_SCORE_CONFIG_REG , &rval);
	galaxyConfig->ctrlState.CLUST_TEMPL_SCORE_CONFIG[sys][contexts][templ]=rval;
      }
    kkk:
      ;
    }
  }
  /* mutexes section *int system , int context , int hc , int pc , int sp , targetE target*/

  return (SUCCESS);

}


/************************************************************************************************
 * Initialize the API
 ************************************************************************************************/

int vtApiInit (galaxyConfigT * galaxyConfig, hostE host, targetE target)
{
  int simStubErr=FAILURE;
  int insizzleErr=FAILURE;
  int siliconErr=FAILURE;
  int rtlErr=FAILURE;
#if VTAPIDEBUG
  printf ("initializing initVtApi \n");
#endif
  ///////////////////////////////////
  // TARGET
  ///////////////////////////////////
  /* Check valid host/target combinations */
  if (target == SIMSTUB) 
    {
      // SIMSTUB is valid for X86 , MB
      if ( (host ==X86) | (host==MB)) {
	//simStubErr=simStubInitVtApi  (galaxyConfig);
	return(simStubErr);
      }
      else
	return (INVALIDHOSTTARGET);
    }
  else if (target == INSIZZLE) 
    {
      // Check error codes (DBGRESP)
      insizzleErr=insizzleStubInitVtApi  (galaxyConfig);
      return(insizzleErr);
    }
  else if (target == RTL) 
    {
      // Check error codes (DBGRESP)
      rtlErr=getGalaxyConfig  (host , target , galaxyConfig);
      return(rtlErr);
    }
  //===========================================================
  /* This is the primary configuration. galaxyConfig is populated with
   * direct transactions to the LE1
   */
  else if (target == SILICON) 
    {
      //siliconErr=siliconInitVtApi  (galaxyConfig);
      if ( (host==MB)) {
	siliconErr=getGalaxyConfig  (MB , SILICON , galaxyConfig);
	return(siliconErr);
      }
      else
	return (INVALIDHOSTTARGET);
    //return(siliconErr);
    }
  else if (target == SYSTEMCTB) 
    {
      //siliconErr=siliconInitVtApi  (galaxyConfig);
      return(siliconErr);
    }
  else
    return (FAILURE);
}

/************************************************************************************************
 * vtApiSetUpInstrumentation: Connect an instrumentation counter to an event
 ************************************************************************************************/
int vtApiSetUpInstrumentation(	galaxyConfigT *galaxyConfig , unsigned int instrPeriphId , unsigned int system , unsigned int context , unsigned int hContext , unsigned int counter , instrTriggerT trigger ,  hostE host, targetE target)
{
  int err=FAILURE;
  unsigned int rdata;
  unsigned int wdata=0;//0xdead + counter;

  vtDbgsetCurrent(system , context , hContext  ,  0 /* cluster */,host , target);
#if VTAPIDEBUG
  printf ("vtApiSetUpInstrumentation: Connecting S[%d]INSTR[%d]CNT[%d] to event C[%d]HC[%d][%d]\n",system, instrPeriphId, counter, context, hContext, trigger);
#endif
  /* Step 1: Write to counter and readback; If succesfull, proceed */
  err=vtApiWrPeriph(galaxyConfig , system , instrPeriphId ,INSTR_COUNTER_REG_BASE + counter, wdata,  target); if (err) return (err);
  err=vtApiRdPeriph(galaxyConfig , system , instrPeriphId ,INSTR_COUNTER_REG_BASE + counter, &rdata, target); if (err) return (err);
  if (wdata!=rdata)  {
    printf ("vtApiSetUpInstrumentation: ERROR in setting CNT[%d]\n",counter);
    return (FAILURE);
  }
  /* Step 2: Write to counter ctrl and readback; If succesfull, proceed */
  wdata|=
    ((context & INSTR_CTRL_REG_CONTEXT_MASK) << INSTR_CTRL_REG_CONTEXT_SHIFT) |
    ((hContext & INSTR_CTRL_REG_HCONTEXT_MASK) << INSTR_CTRL_REG_HCONTEXT_SHIFT) |

    ((0 & INSTR_CTRL_REG_CONTEXT_MASK) << INSTR_CTRL_REG_CONTEXT_SHIFT) |
    ((0 & INSTR_CTRL_REG_HCONTEXT_MASK) << INSTR_CTRL_REG_HCONTEXT_SHIFT) |
    ((trigger & INSTR_CTRL_REG_TRIGGER_MASK) << INSTR_CTRL_REG_TRIGGER_SHIFT) |
    (1 << INSTR_CTRL_REG_ENABLED_SHIFT) |
    (0 << INSTR_CTRL_REG_INTERNAL_SHIFT) |
    ((0 << INSTR_CTRL_REG_LINKED_MASK) << INSTR_CTRL_REG_LINKED_MASK);
  printf ("WRITING %x in 0x%x\n",wdata, INSTR_CTRL_REG_BASE + counter);
  err=vtApiWrPeriph(galaxyConfig , system , instrPeriphId ,INSTR_CTRL_REG_BASE + counter, wdata,  target); if (err) return (err);
  err=vtApiRdPeriph(galaxyConfig , system , instrPeriphId ,INSTR_CTRL_REG_BASE + counter, &rdata, target); if (err) return (err);
  if (wdata!=rdata)  {
    printf ("vtApiSetUpInstrumentation: ERROR in setting INSTR_CTRL_REG_BASE[%d]. Wrote %x and read back %x\n",counter,wdata,rdata);
    //return (FAILURE);
  }
  return (SUCCESS);

}

/************************************************************************************************
 * vtApiRdInstrumentation: Return the contents (long long) of an instrumentation counter
 ************************************************************************************************/
int vtApiRdInstrumentation(	galaxyConfigT *galaxyConfig , unsigned int instrPeriphId , unsigned int system , unsigned int context , unsigned int hContext , unsigned int counter , unsigned long  * rdata ,  instrTriggerT instrTrigger, hostE host, targetE target)
{
  int err=FAILURE;
  unsigned int rdataCnt, rdataCtrl;
  unsigned long  rdataLL=0;

  unsigned int wdata=0;//0xdead + counter;

  vtDbgsetCurrent(system , context , hContext  ,  0 /* cluster */,host , target);
#if VTAPIDEBUG
  //printf ("vtApiRdInstrumentation: Connecting S[%d]INSTR[%d]CNT[%d] to event C[%d]HC[%d][%d]\n",system, instrPeriphId, counter, context, hContext, trigger);
#endif
  /* Step 1: Read ctrl register  */
  err=vtApiRdPeriph(galaxyConfig , system , instrPeriphId ,INSTR_CTRL_REG_BASE + counter, &rdataCtrl, target); if (err) return (err);
  /* Step 2: Read counter register  */
  err=vtApiRdPeriph(galaxyConfig , system , instrPeriphId ,INSTR_COUNTER_REG_BASE + counter, &rdataCnt, target); if (err) return (err);
  rdataLL=( long) rdataCnt;
  //printf ("1-rdataLL=%x\n",rdataLL);
  // If counter overflowed then increment by 2**32
  if ((rdataCtrl>>INSTR_CTRL_REG_OVF_SHIFT) & INSTR_CTRL_REG_OVF_MASK) {
    rdataLL +=  (( long) (2))<<32 ;
  }
  *rdata=rdataLL;//rdataLL;
  //printf ("2-rdataLL=%x\n",rdataLL);
  return (SUCCESS);

}


/************************************************************************************************
 * Load one IRAM location of a selected S.C with data
 ************************************************************************************************/
int wrOneIramLocation (galaxyConfigT *galaxyConfig , unsigned int system , unsigned int context ,unsigned  int iaddr , unsigned int data, targetE target)
{
  int err=FAILURE;
  unsigned int rval;
  if (target == SIMSTUB) {
    //if (err=simStubWrOneIramLocation (galaxyConfig , system , context , iaddr , data)) exit(err);
  }
  else if (target == INSIZZLE) {
    insizzleSetCurrent(system , context,  0 ,  0);
    return(insizzleWrOneIramLocation (galaxyConfig, iaddr , data));
  }
  else if ((target == SILICON) | (target == SYSTEMCTB) | (target == RTL)  ) {
    if (VT_RM_DEBUG_IF) {
      // Use the new DEBUG I/F
    }
    else {

      // Legacy DEBUG I/F
#if VTAPIDEBUG
      printf ("wrOneIramLocation: Addr%x=%x\n",iaddr,data);
#endif
      vtDbgsetCurrent(system , context,  0 ,  0 ,NOHOST , target);
      vtDbgWrIf (ADDR_REG , IRAM_DBG_BASE + iaddr, target);
      vtDbgWrIf (WRDATA1_REG , data, target);
      vtDbgWrIf (CMD_REG , DBGCMD_LEGACY_WR, target);
      vtBusyWait(target);
      return (SUCCESS);

    }

    //setCurrent
  }
}

/************************************************************************************************
 * Load one DRAM location of a selected S with data
 ************************************************************************************************/
int wrOneDramLocation (galaxyConfigT *galaxyConfig ,unsigned  int system , unsigned int daddr , unsigned int data, targetE target)
{
  int err=FAILURE;
  if (target == SIMSTUB) {
    //if (err=simStubWrOneDramLocation (galaxyConfig , system , daddr , data)) exit(err);
  }
  else if (target == INSIZZLE) {
    // Call
    insizzleSetCurrent(system , 0,  0 ,  0);
    insizzleWrOneDramLocation (galaxyConfig , daddr , swapByte(data));
  }
  else if ((target == SILICON) | (target == SYSTEMCTB)  | (target == RTL) ) {
    if (VT_RM_DEBUG_IF) {
      // Use the new DEBUG I/F
    }
    else {
      // Legacy DEBUG I/F
      //printf ("wrOneIramLocation: Addr%x=%x\n",iaddr,data);
      vtDbgWrIf (ADDR_REG , DRAM_DBG_BASE + daddr, target);
      vtDbgWrIf (WRDATA1_REG , data, target);
      vtDbgWrIf (CMD_REG , DBGCMD_LEGACY_WR, target);
      vtBusyWait(target);
      return (SUCCESS);
    }

    //setCurrent
  }
}

/************************************************************************************************
 * Get one IRAM location from selected S.C
 ************************************************************************************************/
int rdOneIramLocation (galaxyConfigT *galaxyConfig, unsigned int system , unsigned int context , unsigned int iaddr , unsigned int * data, targetE target)
{
  int err;
  unsigned int rval;
  if (target == SIMSTUB) {
    //	if (err=simStubRdrOneIramLocation (galaxyConfig , system , context , iaddr , data)) exit(err);
  }
  else if (target == INSIZZLE) {
    insizzleSetCurrent(system , context,  0 ,  0);
    return(insizzleRdOneIramLocation (galaxyConfig , iaddr , (int*)data));
  }
  else if ((target == SILICON) | (target == SYSTEMCTB)  | (target == RTL)  ) {
    if (VT_RM_DEBUG_IF) {
      // Use the new DEBUG I/F
    }
    else {
      // Legacy DEBUG I/F

      vtDbgWrIf (ADDR_REG , IRAM_DBG_BASE + iaddr, target);
      vtDbgWrIf (CMD_REG , DBGCMD_LEGACY_RD, target);
      vtBusyWait(target);
      vtDbgRdIf(RDDATA1_REG, &rval , target);
      *data=rval;
      //printf ("rdOneIramLocation (%x)=%x\n",iaddr,rval);
      return (SUCCESS);

    }

    //setCurrent
  }

}
/************************************************************************************************
 * Get one DRAM location from selected S.C
 ************************************************************************************************/
int rdOneDramLocation (galaxyConfigT *galaxyConfig, unsigned int system ,  unsigned int daddr , unsigned int * data, targetE target)
{
  int err;
  unsigned int rval;
  if (target == SIMSTUB) {
    //	if (err=simStubRdrOneDramLocation (galaxyConfig , system , daddr , data)) exit(err);
  }
  else if (target == INSIZZLE) {
    insizzleSetCurrent(system ,0,  0 ,  0);
    return(insizzleRdOneDramLocation (galaxyConfig , daddr , (int *)data));
  }
  else if ((target == SILICON) | (target == SYSTEMCTB)  | (target == RTL)   ) {
    if (VT_RM_DEBUG_IF) {
      // Use the new DEBUG I/F
    }
    else {
      // Legacy DEBUG I/F
      vtDbgWrIf (ADDR_REG , DRAM_DBG_BASE + daddr, target);
      vtDbgWrIf (CMD_REG , DBGCMD_LEGACY_RD, target);
      vtBusyWait(target);
      vtDbgRdIf(RDDATA1_REG, &rval , target);
      *data=rval;
      //printf ("rdOneIramLocation (%x)=%x\n",daddr,rval);
      return (SUCCESS);

    }

    //setCurrent
  }

}

/************************************************************************************************
 * Load the IRAM of a given context with thsetCurrente array iramBin
 * Destroys the contents of the SYSTEM_REG/CONTEXT_REG/HYPERCONTEXT_REG/CLUSTER_REG
 * for SILICON target
 ************************************************************************************************/
int vtApiLoadIram(galaxyConfigT *galaxyConfig, unsigned int system , unsigned int context , char * iramBin , unsigned int iramLen, hostE host , targetE target)
{
  unsigned int  rdata ,  wdata,addr;
  int i, err;
  char *wdataP;
  err=FAILURE;
  // Init char ptr
  wdataP=(char *) &wdata;
  vtDbgsetCurrent(system , context , 0  /* hc */,  0 /* cluster */,host , target);
  for (i=0;i<iramLen;i+=4) {
    // Write to IRAM location i the value wdata. Notice the ENDIANESS SHIFT
    *wdataP=iramBin[i+0];*(wdataP+1)=iramBin[i+1];*(wdataP+2)=iramBin[i+2];*(wdataP+3)=iramBin[i+3];
#ifdef SCTB		// LE->BE
    if (err=wrOneIramLocation (galaxyConfig, system , context , i, swapByte(wdata), target)) exit (err);
    if (err=rdOneIramLocation (galaxyConfig, system , context , i, &rdata, target)) exit (err);
    if (swapByte(rdata) != wdata) exit(IRAMCYCLEERROR);
    //if (swapByte(rdata) != swapByte(wdata)) exit(IRAMCYCLEERROR);
#else			// BE->BE
    if (err=wrOneIramLocation (galaxyConfig, system , context , i, wdata, target)) exit (err);
    if (err=rdOneIramLocation (galaxyConfig, system , context , i, &rdata, target)) exit (err);
    if (rdata != wdata) exit(IRAMCYCLEERROR);
#endif

  }
#if VTAPIDEBUG
  printf ("vtApiLoadIram: IRAM[%d][%d] loaded with %d bytes\n",system , context , iramLen);
#endif
  return (SUCCESS);
}
/************************************************************************************************
 * Load the DRAM of a given SYSTEM with the array dramBin
 * Destroys the contents of the SYSTEM_REG for SILICON target
 ************************************************************************************************/
int vtApiLoadDram(galaxyConfigT *galaxyConfig , unsigned int system , char * dramBin, unsigned int dramLen , hostE host, targetE target)
{
  unsigned int  rdata ,  wdata,addr;
  int i , err;
  char *wdataP;
  err=FAILURE;
  // Init char ptr
  wdataP= (char *) &wdata;
  vtDbgsetCurrent(system , 0 /* context */, 0  /* hc */,  0 /* cluster */,host , target);
  for (i=0;i<dramLen;i+=4) {
    // Write to DRAM location i the value wdata. Notice the ENDIANESS SHIFT
    *wdataP=dramBin[i+0];*(wdataP+1)=dramBin[i+1];*(wdataP+2)=dramBin[i+2];*(wdataP+3)=dramBin[i+3];
#ifdef SCTB			// LE->BE
    if (err=wrOneDramLocation (galaxyConfig, system , i, swapByte(wdata), target)) exit (err);
    if (err=rdOneDramLocation (galaxyConfig, system , i, &rdata, target)) exit (err);
    if (swapByte(rdata) != wdata) exit(DRAMCYCLEERROR);
    //if (swapByte(rdata) != swapByte(wdata)) exit(DRAMCYCLEERROR);
#else				// BE->BE
    if (err=wrOneDramLocation (galaxyConfig, system , i, wdata, target)) exit (err);
    if (err=rdOneDramLocation (galaxyConfig, system , i, &rdata, target)) exit (err);
    if (rdata != wdata) exit(DRAMCYCLEERROR);
#endif
  }
#if VTAPIDEBUG
  printf ("vtApiLoadDram: DRAM[%d]loaded with %d bytes\n",system , dramLen);
#endif
  return (SUCCESS);
}

/************************************************************************************************
 * Sets CTRL_REG.STATUS of the addressed S.C.HC to the value specified
 ************************************************************************************************/
int wrCtrlState(galaxyConfigT *galaxyConfig ,unsigned  int system , unsigned int context , unsigned int hc , vtCtrlStateE vtCtrlState, hostE host , targetE target)
{
  int err=FAILURE;
  currentT current;
  if (target == SIMSTUB) {
    //int simStubSetCtrlStatus		(galaxyConfigT *galaxyConfig , int system , int context , int hc , vtCtrlStatusE vtCtrlStatus);
    //if (err=simStubWrCtrlState (galaxyConfig , system , context, hc , RUNNING)) exit(err);
  }
  else if (target == INSIZZLE) {
    //insizzleWrOneIramLocation (galaxyConfig , system , context , iaddr , data);
  }
  else if ((target == SILICON) | (target == SYSTEMCTB)  | (target == RTL)  ) {
    if (VT_RM_DEBUG_IF) {
      // Use the new DEBUG I/F
    }
    else {
      // Legacy DEBUG I/F
      //vtDbgWr (ADDR_REG , daddr, target);
      //vtDbgWr (CMD_REG , DBG_RD, target);
    }

    //setCurrent
  }

  return (err);
}

/************************************************************************************************
 * Reads the CTRL_REG.STATUS of the addressed S.C.HC
 ************************************************************************************************/
int rdCtrlState(galaxyConfigT *galaxyConfig , unsigned int system , unsigned int context , unsigned int hc , vtCtrlStateE *vtCtrlState, hostE host , targetE target)
{
  int err=FAILURE;
  currentT current;
  if (target == SIMSTUB) {
    //int simStubSetCtrlStatus		(galaxyConfigT *galaxyConfig , int system , int context , int hc , vtCtrlStatusE vtCtrlStatus);
    //if (err=simStubRdCtrlState (galaxyConfig , system , context, hc , vtCtrlState)) exit(err);
  }
  else if (target == INSIZZLE) {
    //insizzleWrOneIramLocation (galaxyConfig , system , context , iaddr , data);
  }
  else if ((target == SILICON) | (target == SYSTEMCTB) | (target == RTL)  ) {
    if (VT_RM_DEBUG_IF) {
      // Use the new DEBUG I/F
    }
    else {
      // Legacy DEBUG I/F
      //vtDbgWr (ADDR_REG , daddr, target);
      //vtDbgWr (CMD_REG , DBG_RD, target);
    }

    //setCurrent
  }

  return (err);
}

/************************************************************************************************
 * Reads one S_GPR from the addressed S.C.HC.CL and store in rdata
 ************************************************************************************************/
int vtApiRdOneSGpr( unsigned int sgpr, unsigned int * rdata, targetE target)
{
  int err=FAILURE;
  unsigned int rval;
  currentT current;
  if (target == SIMSTUB) {
    //if (err=(simStubRdOneSGpr(galaxyConfig, system , context, hypercontext, cluster, sgpr, rdata))) exit(err);
  }
  else if (target == INSIZZLE) {
    //if (err=(insizzleRdOneSGpr(s , c, hc, cl, host , target, &rdata))) exit(err);
  }
  else if ((target == SILICON) | (target == SYSTEMCTB)  | (target == RTL)  ) {
    if (VT_RM_DEBUG_IF) {
      // Use the new DEBUG I/F
    }
    else {
      // Legacy DEBUG I/F
      /* 			dbg_wr(ADDR_REG,S_GPR_DBG_BASE + reg * 4);
				dbg_wr (CMD_REG , DBG_RD);
				busy_wait();
				return(SUCCESS);*/


      vtDbgWrIf (ADDR_REG , S_GPR_DBG_BASE + sgpr * 4, target);
      vtDbgWrIf (CMD_REG , DBGCMD_LEGACY_RD, target);
      vtBusyWait(target);
      vtDbgRdIf(RDDATA1_REG, &rval , target);
      *rdata=rval;
      //printf ("rdOneIramLocation (%x)=%x\n",iaddr,rval);
      return (SUCCESS);
    }

    //setCurrent
  }

  return (err);
}

/************************************************************************************************
 * vtApiRdPeriph: Read one register in the addressed peripheral
 ************************************************************************************************/
int vtApiRdPeriph(galaxyConfigT *galaxyConfig , unsigned int system , unsigned int periphId ,unsigned int reg, unsigned int *rdata, targetE target)
{
  int err=FAILURE;
  unsigned int rval;
  currentT current;
  if (target == SIMSTUB) {
    //if (err=(simStubRdOneSGpr(galaxyConfig, system , context, hypercontext, cluster, sgpr, rdata))) exit(err);
  }
  else if (target == INSIZZLE) {
    //if (err=(insizzleRdOneSGpr(s , c, hc, cl, host , target, &rdata))) exit(err);
  }
  else if ((target == SILICON) | (target == SYSTEMCTB) | (target == RTL)  ) {
    if (VT_RM_DEBUG_IF) {
      // Use the new DEBUG I/F
    }
    else {
      // Legacy DEBUG I/F
      /* 			dbg_wr(ADDR_REG,S_GPR_DBG_BASE + reg * 4);
				dbg_wr (CMD_REG , DBG_RD);
				busy_wait();
				return(SUCCESS);*/

      vtDbgWrIf (ADDR_REG , PERIPH_DBG_BASE + reg, target);
      vtDbgWrIf (CMD_REG , DBGCMD_LEGACY_RD, target);
      vtBusyWait(target);
      vtDbgRdIf(RDDATA1_REG, &rval , target);
      *rdata=rval;
#if VTAPIDEBUG
      printf ("vtApiRdPeriph: Read %x for  R[%d] at [%x] \n",rval, reg , PERIPH_DBG_BASE+reg);
#endif

      return (SUCCESS);
    }


  }

  return (err);
}

/************************************************************************************************
 * vtApiWrPeriph: Write one register in the addressed peripheral
 ************************************************************************************************/
int vtApiWrPeriph(galaxyConfigT *galaxyConfig , unsigned int system , unsigned int periphId ,unsigned int reg, unsigned int wdata,  targetE target)
{
  int err=FAILURE;
  unsigned int rval;
  currentT current;
  if (target == SIMSTUB) {
    //if (err=(simStubRdOneSGpr(galaxyConfig, system , context, hypercontext, cluster, sgpr, rdata))) exit(err);
  }
  else if (target == INSIZZLE) {
    //if (err=(insizzleRdOneSGpr(s , c, hc, cl, host , target, &rdata))) exit(err);
  }
  else if ((target == SILICON) | (target == SYSTEMCTB)  | (target == RTL)  ) {
    if (VT_RM_DEBUG_IF) {
      // Use the new DEBUG I/F
    }
    else {
      // Legacy DEBUG I/F
      /* 			dbg_wr(ADDR_REG,S_GPR_DBG_BASE + reg * 4);
				dbg_wr (CMD_REG , DBG_RD);
				busy_wait();
				return(SUCCESS);*/


      vtDbgWrIf (ADDR_REG , PERIPH_DBG_BASE + reg, target);
      vtDbgWrIf (WRDATA1_REG , wdata, target);
      vtDbgWrIf (CMD_REG , DBGCMD_LEGACY_WR, target);
      vtBusyWait(target);
#if VTAPIDEBUG
      printf ("vtApiWrPeriph: Wrote R[%d] at [%x] with %x\n",reg , PERIPH_DBG_BASE+reg , wdata);
#endif
      return (SUCCESS);
    }

    //setCurrent
  }

  return (err);
}

/************************************************************************************************
 * Reads one S_GPR from the addressed S.C.HC.CL and store in rdata
 ************************************************************************************************/
int vtApiWrOneSGpr(unsigned int sgpr , unsigned int  wdata, targetE target)
{
  int err=FAILURE;
  unsigned int rval;
  currentT current;
  if (target == SIMSTUB) {
    //if (err=(simStubRdOneSGpr(galaxyConfig, system , context, hypercontext, cluster, sgpr, rdata))) exit(err);
  }
  else if (target == INSIZZLE) {
    //if (err=(insizzleRdOneSGpr(s , c, hc, cl, host , target, &rdata))) exit(err);
  }
  else if ((target == SILICON) | (target == SYSTEMCTB)  | (target == RTL)  ) {
    if (VT_RM_DEBUG_IF) {
      // Use the new DEBUG I/F
    }
    else {
      // Legacy DEBUG I/F
      /* 			dbg_wr(ADDR_REG,S_GPR_DBG_BASE + reg * 4);
				dbg_wr (CMD_REG , DBG_RD);
				busy_wait();
				return(SUCCESS);*/


      vtDbgWrIf (ADDR_REG , S_GPR_DBG_BASE + sgpr * 4, target);
      vtDbgWrIf (WRDATA1_REG , wdata, target);
      vtDbgWrIf (CMD_REG , DBGCMD_LEGACY_WR, target);
      vtBusyWait(target);

      return (SUCCESS);
    }

    //setCurrent
  }

  return (err);
}

/************************************************************************************************
 * Reads one S_GPR from the addressed S.C.HC.CL and store in rdata
 ************************************************************************************************/
int rdOneSPr(galaxyConfigT *galaxyConfig , unsigned int system , unsigned int context , unsigned int hypercontext , unsigned int cluster, unsigned int spr, hostE host , targetE target , unsigned int * rdata)
{
  int err=FAILURE;
  currentT current;
  if (target == SIMSTUB) {
    //	if (err=(simStubRdOneSPr(galaxyConfig, system , context, hypercontext, cluster, spr, rdata))) exit(err);
  }
  else if (target == INSIZZLE) {
    //if (err=(insizzleRdOneSGpr(s , c, hc, cl, host , target, &rdata))) exit(err);
  }
  else if ((target == SILICON) | (target == SYSTEMCTB) | (target == RTL)  ) {
    if (VT_RM_DEBUG_IF) {
      // Use the new DEBUG I/F
    }
    else {
      // Legacy DEBUG I/F
      //vtDbgWr (ADDR_REG , daddr, target);
      //vtDbgWr (CMD_REG , DBG_RD, target);
    }

    //setCurrent
  }

  return (err);
}

/************************************************************************************************
 * returns the PC of the addressed S.C.HC
 ************************************************************************************************/
int rdOnePC(galaxyConfigT *galaxyConfig , unsigned int system , unsigned int context , unsigned int hypercontext , hostE host , targetE target , int * rdata)
{
  int err=FAILURE;
  currentT current;
  if (target == SIMSTUB) {
    //	if (err=(simStubRdOnePC(galaxyConfig, system , context, hypercontext, rdata))) exit(err);
  }
  else if (target == INSIZZLE) {
    //if (err=(insizzleRdOneSGpr(s , c, hc, cl, host , target, &rdata))) exit(err);
  }
  else if ((target == SILICON) | (target == SYSTEMCTB) | (target == RTL)  ) {
    if (VT_RM_DEBUG_IF) {
      // Use the new DEBUG I/F
    }
    else {
      // Legacy DEBUG I/F
      //vtDbgWr (ADDR_REG , daddr, target);
      //vtDbgWr (CMD_REG , DBG_RD, target);
    }

    //setCurrent
  }

  return (err);
}

/************************************************************************************************
 * returns the PC of the addressed S.C.HC
 ************************************************************************************************/
int rdOneLr(galaxyConfigT *galaxyConfig , unsigned int system , unsigned int context , unsigned int hypercontext , hostE host , targetE target , unsigned int * rdata)
{
  int err=FAILURE;
  currentT current;
  if (target == SIMSTUB) {
    //if (err=(simStubRdOneLr(galaxyConfig, system , context, hypercontext, rdata))) exit(err);
  }
  else if (target == INSIZZLE) {
    //if (err=(insizzleRdOneSGpr(s , c, hc, cl, host , target, &rdata))) exit(err);
  }
  else if (target == SILICON)  {
    if (VT_RM_DEBUG_IF) {
      // Use the new DEBUG I/F
    }
    else {
      // Legacy DEBUG I/F
      //vtDbgWr (ADDR_REG , daddr, target);
      //vtDbgWr (CMD_REG , DBG_RD, target);
    }
    //setCurrent
  }
  else if  (target == SYSTEMCTB) {


  }

  return (err);
}
/************************************************************************************************
 * Writes to a special register
 ************************************************************************************************/
void vtApiWrSpecial(unsigned int special_reg , unsigned int data, targetE target)
{
  vtDbgWrIf (ADDR_REG , SPECIAL_DBG_BASE + special_reg * 4, target);
  vtDbgWrIf (WRDATA1_REG , data, target);
  vtDbgWrIf (CMD_REG , DBGCMD_LEGACY_WR, target);
  vtBusyWait(target);
#if VTAPIDEBUG
  printf ("write_special: Issued write on %d with value %x\n",special_reg , data);
#endif

  /*  dbg_wr(ADDR_REG , SPECIAL_DBG_BASE + special_reg * 4);
      dbg_wr(WRDATA1_REG , data);
      dbg_wr(CMD_REG , DBG_WR);
      printf ("write_special: Issued write on %d with value %x\n",special_reg , data);*/
};

/************************************************************************************************
 * Busy-waits on the VT_CTRL_REG of the addressed S.C.HC until DEBUG=1
 ************************************************************************************************/
void vtApiCtrlWait(hostE host , targetE target)
{
  unsigned int reg;
  unsigned long  iter=0;
#if VTAPIDEBUG
  printf ("entering ctrlwait loop...\n");
#endif
  do  {
    // extract control reg
    //reg = read_cr();
    vtDbgRdCtrl(host , target, CTRL_REG , &reg);
    //printf ("%x\n",reg);
    iter++;
    if ((iter % (1024 * 1024 ) == 0))   printf  ("%luM iterations. CTRL_REG=%x\n",iter/(1024*1024),reg);  }
  while (  !((reg>>1) & 0x1)  );
  printf  ("Busy-waited for %lu iterations...\n",iter);
}

/************************************************************************************************
 * Starts the addressed hypercontext (STATE=RUNNING) at given PC with given SP
 ************************************************************************************************/
int vtApiStartOneHc(galaxyConfigT *galaxyConfig, unsigned int system , unsigned int context , unsigned int hc , unsigned int pc , unsigned int sp , hostE host , targetE target)
{
  int err=FAILURE;
  unsigned int spval;
  unsigned int rval;

  vtDbgsetCurrent(system , context, hc, 0, host , target) ;
  vtApiWrOneSGpr(SP , sp, target );
  vtApiRdOneSGpr(SP,&spval,target);
  //if (!(target ==0x1f000))
  {
#if VTAPIDEBUG
    printf ("vtApiStartOneHc: SP passed  as %x)\n",spval);
#endif
    //exit(0);
  }
  // Setup PC
  vtDbgRdCtrl(host , target, CTRL_REG , &rval);
#if VTAPIDEBUG
  printf ("DBGCTRL_REG (before writing PC) =%x\n",rval);
#endif
  vtApiWrSpecial (PC_REG , 0x0, target);
  // print contents of dbgctrl_reg
  vtDbgRdCtrl(host , target, CTRL_REG , &rval);
#if VTAPIDEBUG
  //printf ("DBGCTRL_REG (after writing PC) =%x\n",rval);
#endif
  // setup DBGCTRL
  vtDbgWrCtrl(host , target, DBGCTRL_REG , DBGCTRL_RUN);
  //write_cr(DBGCTRL_REG , DBGCTRL_RUN);
  vtDbgRdCtrl(host , target, CTRL_REG , &rval);
#if VTAPIDEBUG
  printf ("DBGCTRL_REG (after writing DBGCTRL) =%x\n",rval);
#endif
  //printf ("Waiting for %s to finish...\n",name);
  //ctrl_wait();
  return (SUCCESS);
}

/************************************************************************************************
 * Waits for the addressed hypercontext to return to the DEBUG state (VTPRM)
 ************************************************************************************************/
int vtApiWaitOneHc(galaxyConfigT *galaxyConfig, unsigned int system , unsigned int context , unsigned int hc ,  hostE host , targetE target)
{
  int err=FAILURE;
  vtApiCtrlWait( host , target);
  return (SUCCESS);
  vtCtrlStateE vtCtrlState;
  do {
    if (err=rdCtrlState(galaxyConfig , system , context , hc , &vtCtrlState , host , target)) exit(err);
  } while (vtCtrlState==RUNNING);
  return(err);
}
/************************************************************************************************
 * Do a memory dump of the DRAM of S
 ************************************************************************************************/
int vtApiDumpDram(galaxyConfigT * galaxyConfig, unsigned int system , unsigned int * dump, hostE host, targetE target)
{
  unsigned int  rdata ,  wdata,addr;
  int i , err;
  unsigned int dramSize=0x20000;
  err=FAILURE;
  // FIXME !!!
  //printf ("vtApiDumpDram for S[%d], size %x\n",system , galaxyConfig->systemConfig[system].dramSize);
  //dramSize= (((galaxyConfig->ctrlState.dram_shared_config0_reg[0]) >> DRAM_SHARED_CONFIG0_REG_DRAM_SIZE_SHIFT) & DRAM_SHARED_CONFIG0_REG_DRAM_SIZE_MASK) * 1024;
#if VTAPIDEBUG
  printf ("vtApiDumpDram: Dumping %dKB\n",dramSize);
#endif
  vtDbgsetCurrent(system , 0 /* context */, 0  /* hc */,  0 /* cluster */,host , target);
  for (i=0; i < dramSize; i+=4) {
    if (err=rdOneDramLocation (galaxyConfig, system , i, &rdata, target)) exit (err);
#if VTAPIDEBUG
    //printf ("vtApiDumpDram: *0x%x=%x\n",i,rdata);
#endif
  }
#if VTAPIDEBUG
  printf ("vtApiDumpDram OK\n");
#endif
  return (SUCCESS);
}
/************************************************************************************************
 * Dump one archState
 ************************************************************************************************/
int vtApiDumpOneArchState(galaxyConfigT *galaxyConfig , unsigned int system , unsigned int context ,unsigned int hypercontext, oneArchStateT * oneArchStateDump , hostE host , targetE target)
{
  unsigned int s,c,hc,cl,i, sgpr , sfpr , spr , svr , rdata;
  int err=FAILURE;
  //for (hc=0;hc<hypercontext;hc++) {
  /*		for (cl=0; cl<galaxyConfig->systemConfig[system].contextConfig[context].hyperContextConfig[hypercontext].hClusters; cl++) {
  // sGPR
  for (sgpr=0;sgpr<galaxyConfig->systemConfig[system].contextConfig[context].hyperContextConfig[hypercontext].sGprFileSize[cl]; sgpr++) {
  if (err=(rdOneSGpr(galaxyConfig, s , c, hc, cl, sgpr, host , target, &rdata))) exit(err);
  oneArchStateDump->sGpr[cl][sgpr]=rdata;
  }

  // sPR
  for (spr=0;spr<galaxyConfig->systemConfig[s].contextConfig[c].hyperContextConfig[hc].sPrFileSize[cl];spr++) {
  if (err=(rdOneSPr(galaxyConfig, s , c, hc, cl, spr, host , target, &rdata))) exit(err);
  oneArchStateDump->sPr[cl][spr]=rdata;
  }

  // pc
  if (err=(rdOnePC(galaxyConfig, s , c, hc,  host , target, &rdata))) exit(err);
  oneArchStateDump->pc=rdata;

  // lr
  if (err=(rdOneLr(galaxyConfig, s , c, hc,  host , target, &rdata))) exit(err);
  oneArchStateDump->lr=rdata;*/
  //		}
  //}
  printf ("vtApiDumpOneArchState NOT IMPLEMENTED\n");
  return (SUCCESS);
}

/************************************************************************************************
 * Dump fullArchState
 ************************************************************************************************/
int vtApiDumpFullArchState(galaxyConfigT *galaxyConfig, unsigned int system , unsigned int context ,unsigned int hypercontext, fullArchStateT * fullArchStateDump , hostE host , targetE target)
{
  unsigned int s,c,hc;
  int err=FAILURE;
  /*	for (s=0;s<galaxyConfig->systems;s++) {
	for (c=0;c<galaxyConfig->systemConfig[s].contexts;c++) {
	for (hc=0;hc<galaxyConfig->systemConfig[s].contextConfig[c].hyperContexts;hc++) {
	//if (err=(vtApiDumpOneArchState(galaxyConfig, s, c, hc, fullArchStateDump[s][c][hc], host, target))) exit(err);
	}
	}
	}*/
  return (SUCCESS);
}



/************************************************************************************************
 * vtApiJoinMultiCHC0
 * Busy wait on HC0 in multiple contexts (0..contexts). Return when all HCs are in DEBUG mode
 ************************************************************************************************/
int vtApiJoinMultiCHC0(galaxyConfigT *galaxyConfig, unsigned int system , unsigned int contexts ,unsigned int contextOffset, hostE host , targetE target)
{
  unsigned int c;
  int err=SUCCESS;
  unsigned int reg;
  long long iter=0;
  if (target == INSIZZLE) {
    /* At this point, the vtapi continuously clocks insizzle. The latter produces the dynamic trace.
       Further processing is needed to ensure that: a) per2gpr insert a reg in inssizle
       b) vthreads commands are proceesed here and state is updated in insizzle under teh
       direct control of the vtapi
    */

    // Declare trace struct
    gTracePacketT  gTracePacket;

    do
      {
	//int s,c,hc;
	// Clock galaxy
	insizzleClock(galaxyConfig , &gTracePacket);
	/*
	  for (s=0;s<1;s++)
	  for (c=0;c<1;c++)
	  for (hc=0;hc<1;hc++)
	*/
	// Perform post-processing (state assignment, GPR hardwiring)
	postClock(galaxyConfig,gTracePacket);
      }
    while (  1  );
  }
  else
    {
      for (c=contextOffset + 0 ;c<contextOffset + contexts;c++) {
	do
	  {
	    iter++;
	    if ((iter % (1024 * 1024 ) == 0)) printf  ("%lluM iterations. CTRL_REG=%x\n",iter/(1024*1024),reg);

	    vtDbgsetCurrent(system , c, 0, 0, host , target) ;
	    // extract control reg
	    vtDbgRdCtrl(host , target, CTRL_REG , &reg);
	  }
	while (  !((reg>>1) & 0x1)  );
#if VTAPIDEBUG
	printf ("vtApiJoinMultiCHC0: C[%d]HC[0] in DEBUG_MODE\n",c);
#endif
      }
    }
  return (SUCCESS);
}


/************************************************************************************************
 * vtApiForkMultiCHC0
 * Start the HC0 in multiple contexts (0..contexts)
 ************************************************************************************************/
int vtApiForkMultiCHC0(galaxyConfigT *galaxyConfig,
		       unsigned int system ,
		       unsigned int contexts ,
		       unsigned int contextOffset,
		       unsigned int pc ,
		       unsigned int spOffset,
		       char * iramBin ,
		       unsigned int iramLen,
		       char * dramBin,
		       unsigned int dramLen ,
		       hostE host ,
		       targetE target)
{
  int c,err;
  err=SUCCESS;
#if VTAPIDEBUG
  printf ("vtApiForkMultiCHC0: Loading DRAM S[%d]\n",system);
#endif
  if (err=
      vtApiLoadDram(	galaxyConfig,	/* galaxyConfigT galaxyConfig */
			0, 				/* int system 	*/
			dramBin, 				/* char * dramBin*/
			dramLen, 		/* int dramLen 	*/
			host,
			target 			/* targetE target*/
			))
    exit (FAILURE);
  for (c=contextOffset + 0 ;c<contextOffset + contexts;c++) {
#if VTAPIDEBUG
    printf ("vtApiForkMultiCHC0: Loading IRAM C[%d]HC[0]\n",c);
#endif
    vtDbgsetCurrent(system , c, 0, 0, host , target) ;
    if (err=
	vtApiLoadIram(	galaxyConfig,	/* galaxyConfigT galaxyConfig */
			0, 				/*int system 	*/
			c, 				/*int context 	*/
			iramBin, 			/*char * iramBin*/
			iramLen, 	/*int iramLen 	*/
			host,			/* hostE host	*/
			target 			/*targetE target*/))
      exit (FAILURE);

  }
  for (c=contextOffset + 0 ;c<contextOffset + contexts;c++) {
    int localstack = STACKTOP-c*spOffset;
#if VTAPIDEBUG
    printf ("vtApiForkMultiCHC0: Starting up C[%d]HC[0]\n",c);
#endif
    if (err=
	vtApiStartOneHc(	galaxyConfig,	/* galaxyConfigT galaxyConfig */
				system, 			/* int system 	*/
				c, 			/* int context 	*/
				0,				/* int hc 		*/
				pc, 			/* int pc 		*/
				localstack,		/*int sp 		*/
				host,
				target	 	/* targetE target*/
				))
      exit (FAILURE);
#if VTAPIDEBUG
    printf ("vtApiForkMultiCHC0: C[%d]HC[0] started\n",c);
#endif
  }
  return (err);
}


/************************************************************************************************
 * vtApiLoadAndStartOneHC
 * Start the addressed S.C.HC
 ************************************************************************************************/
int vtApiLoadAndStartOneHC(
			   galaxyConfigT *galaxyConfig, unsigned int system , unsigned int context ,  unsigned int hypercontext,
			   unsigned int pc , unsigned int spOffset,  char * iramBin , unsigned int iramLen, char * dramBin, unsigned int dramLen ,
			   hostE host , targetE target)
{
  int c,err;
  err=SUCCESS;
#if VTAPIDEBUG
  printf ("vtApiLoadAndStartOneHC: Loading DRAM S[%d]\n",system);
#endif
  if (err=
      vtApiLoadDram(	galaxyConfig,	/* galaxyConfigT galaxyConfig */
			0, 				/* int system 	*/
			dramBin, 				/* char * dramBin*/
			dramLen, 		/* int dramLen 	*/
			host,
			target 			/* targetE target*/
			))
    exit (FAILURE);

#if VTAPIDEBUG
  printf ("vtApiLoadAndStartOneHC: Loading IRAM S[%d]C[%d]\n",system, context);
#endif
  vtDbgsetCurrent(system , context, hypercontext, 0, host , target) ;
  if (err=
      vtApiLoadIram(	galaxyConfig,	/* galaxyConfigT galaxyConfig */
			system, 				/*int system 	*/
			context, 				/*int context 	*/
			iramBin, 			/*char * iramBin*/
			iramLen, 	/*int iramLen 	*/
			host,			/* hostE host	*/
			target 			/*targetE target*/))
    exit (FAILURE);

#if VTAPIDEBUG
  printf ("vtApiLoadAndStartOneHC: Starting up S[%d]C[%d]HC[%d]\n",system, context, hypercontext);
#endif
  if (err=
      vtApiStartOneHc(	galaxyConfig,	/* galaxyConfigT galaxyConfig */
			system, 			/* int system 	*/
			context, 			/* int context 	*/
			hypercontext,				/* int hc 		*/
			pc, 			/* int pc 		*/
			STACKTOP,		/*int sp 		*/
			host,
			target	 	/* targetE target*/
			))
    exit (FAILURE);
#if VTAPIDEBUG
  printf ("vtApiLoadAndStartOneHC: C[%d]HC[0] started\n",c);
#endif

  return (err);
}

/************************************************************************************************
 * startTarget: Called from the default driver; It initializes the system (target-independent) and start  up
 * all targets
 ************************************************************************************************/
int startTargets(galaxyConfigT *galaxyConfig)
{



}


/************************************************************************************************
 * vtApiPrintStats
 * Prints statistics from the addressed target
 ************************************************************************************************/
int vtApiPrintStats(galaxyConfigT *galaxyConfig, statsT* stats, targetE target)
{
  int sys, contexts, hcontexts;
  // This prints out stats for all S.C.HC. These are recovered from the galaxyConfig
  for (sys=0 ; sys < galaxyConfig->ctrlState.GALAXY_CONFIG & GALAXY_CONFIG_REG_SYSTEMS_MASK; sys++) {
    for (contexts=0; contexts<(galaxyConfig->ctrlState.SYSTEM_CONFIG[sys] & SYSTEM_CONFIG_REG_CONTEXTS_MASK); contexts++) {
      for (hcontexts=0 ; hcontexts < (((galaxyConfig->ctrlState.CONTEXT_CONFIG[sys][contexts]) >>CONTEXT_CONFIG_REG_HCONTEXTS_SHIFT) & CONTEXT_CONFIG_REG_HCONTEXTS_MASK) ; hcontexts++) {
	// HC
	printf ("________________________________\n");
	printf ("S[%d]C[%d]HC[%d]TRIGGERS\n",sys, contexts, hcontexts);
	printf ("________________________________\n");
	//printf ("cClocks=%d\n",cClocks);
	printf ("HC_COMMITED_INSTRUCTIONS=%lu\n",stats->hcStats[sys][contexts][hcontexts].HC_COMMITED_INSTRUCTIONS);// hcCommitedInstructions);
	//printf ("HC_STATE_RUNNING_CLOCKS=%d (%f PC of systemStateRunningClocks)\n",stats->hcStats[sys][contexts][hcontexts].HC_STATE_RUNNING_CLOCKS , (float)(100.00 * (float) hcStateRunningClocks  /   (float) systemStateRunningClocks));
	printf ("HC_STATE_RUNNING_CLOCKS=%lu\n",stats->hcStats[sys][contexts][hcontexts].HC_STATE_RUNNING_CLOCKS);
	printf ("HC_PIPE_RESTART_BRANCH=%lu\n",stats->hcStats[sys][contexts][hcontexts].HC_PIPE_RESTART_BRANCH);
	printf ("HC_PIPE_RESTART_CALL=%lu\n",stats->hcStats[sys][contexts][hcontexts].HC_PIPE_RESTART_CALL);
	printf ("HC_PIPE_RESTART_RET=%lu\n",stats->hcStats[sys][contexts][hcontexts].HC_PIPE_RESTART_RET);
	printf ("HC_IFEI_STALL=%lu\n",stats->hcStats[sys][contexts][hcontexts].HC_IFEI_STALL);
	printf ("HC_IFE_NO_FETCH=%lu\n",stats->hcStats[sys][contexts][hcontexts].HC_IFE_NO_FETCH);// , (float)(100.00 * (float)hcIfeNoFetch/(float)hcStateRunningClocks));
	//printf ("HC_IFE_NO_FETCH=%d (%f PC of hcStateRunningClocks)\n",stats->hcStats[sys][contexts][hcontexts].HC_IFE_NO_FETCH , (float)(100.00 * (float)hcIfeNoFetch/(float)hcStateRunningClocks));
	printf ("HC_LSU_HOLD=%lu\n",stats->hcStats[sys][contexts][hcontexts].HC_LSU_HOLD);
	printf ("HC_SCRBD_S_GPR_STALL=%lu\n",stats->hcStats[sys][contexts][hcontexts].HC_SCRBD_S_GPR_STALL);
	printf ("HC_SCRBD_S_BR_STALL=%lu\n",stats->hcStats[sys][contexts][hcontexts].HC_SCRBD_S_BR_STALL);
	printf ("HC_SCRBD_S_BR_STALL=%lu\n",stats->hcStats[sys][contexts][hcontexts].HC_SCRBD_S_BR_STALL);
	printf ("HC_SCRBD_STALL=%lu\n",stats->hcStats[sys][contexts][hcontexts].HC_SCRBD_STALL);
	printf ("HC_SYSTEM_STATE_RUNNING_OFFSET=%lu\n",stats->hcStats[sys][contexts][hcontexts].HC_SYSTEM_STATE_RUNNING_OFFSET);
	printf ("-------------------\n");
	printf ("DERIVED PARAMETERS\n");
	printf ("-------------------\n");
	printf ("CPI=%f\n",(float)SYSTEM_STATE_RUNNING_CLOCKS / HC_COMMITED_INSTRUCTIONS);
	//printf ("CPI (Perfect IFE)=%f\n",(float)((float)hcStateRunningClocks-(float)hcIfeNoFetch)/(float)hcCommitedInstructions);
	//printf ("CPI (Perfect IFE and branching)=%f\n",(float)(((float)hcStateRunningClocks-(float)hcIfeNoFetch-(((float)hcPipeRestartBranch +(float)hcPipeRestartCall+(float)hcPipeRestartReturn )*(float)4))/(float)hcCommitedInstructions));
	//printf ("________________________________\n");
      }
      printf ("-------------------\n");
      //printf ("SYSTEM_STATE_RUNNING_CLOCKS=%lu\n",stats->hcStats[sys][contexts][hcontexts].SYSTEM_STATE_RUNNING_CLOCKS);
      printf ("SYSTEM_STATE_RUNNING_CLOCKS=%lu\n",stats->sStats[sys].SYSTEM_STATE_RUNNING_CLOCKS);
    }
  }
}
/************************************************************************************************
 * vtApiextractStats
 * Extracts statistics from the addressed target
 ************************************************************************************************/
int vtApiextractStats(statsT* stats, galaxyConfigT *galaxyConfig, hostE host , targetE target)
{
  int c,err;
  err=SUCCESS;
      // Declare these locals to store the contentes of the counters
      unsigned long  cClocks=0;
      unsigned long  hcCommitedInstructions=0;
      unsigned long  hcStateRunningClocks=0;
      unsigned long  hcPipeRestartBranch=0;
      unsigned long  hcPipeRestartCall=0;
      unsigned long  hcPipeRestartReturn=0;
      unsigned long  hcIfeNoFetch=0;
      unsigned long  hcLsuHold=0;
      unsigned long  hcIfeiStall=0;
      unsigned long  hcScrbdSGprStall=0;
      unsigned long  hcScrbdSBrStall=0;
      unsigned long  hcScrbdSLrStall=0;
      unsigned long  hcScrbdStall=0;
      unsigned long  systemStateRunningClocks=0;//SYSTEM_STATE_RUNNING_CLOCKS
      unsigned long  hcSystemStateRunningOffset=0;//HC_SYSTEM_STATE_RUNNING_OFFSET
#if VTAPIDEBUG
  printf ("In vtApiextractStats\n");
#endif
  /*  switch (host)
    {
    case X86:
  */
      if (host==X86) {

      /* ######################################
       * Step 8
       * At this point, the application can
       * execute on the host system (X86/MB).
       * The output arrays can be compared
       * against the memDump produced
       * by the VThreads HC
       ########################################*/
      {
	// Declare these locals to store the contentes of the counters
	unsigned long  cClocks=0;
	unsigned long  hcCommitedInstructions=0;
	unsigned long  hcStateRunningClocks=0;
	unsigned long  hcPipeRestartBranch=0;
	unsigned long  hcPipeRestartCall=0;
	unsigned long  hcPipeRestartReturn=0;
	unsigned long  hcIfeNoFetch=0;
	unsigned long  hcLsuHold=0;
	unsigned long  hcIfeiStall=0;
	unsigned long  hcScrbdSGprStall=0;
	unsigned long  hcScrbdSBrStall=0;
	unsigned long  hcScrbdSLrStall=0;
	unsigned long  hcScrbdStall=0;
	unsigned long  systemStateRunningClocks=0;//SYSTEM_STATE_RUNNING_CLOCKS
	unsigned long  hcSystemStateRunningOffset=0;//HC_SYSTEM_STATE_RUNNING_OFFSET


 	/* ######################################
   	 * Step_1 :
   	 * Report counter15
   	 ######################################## */
   	if (err=
	    vtApiRdInstrumentation
	    (galaxyConfig,	/* galaxyConfigT galaxyConfig */
	     0 ,				/* instrPeriphId */
	     0, 				/* int system 	*/
	     0, 				/* int context 	*/
	     0,				/* int hContext */
	     15,				/* int counter */
	     (long unsigned int*) &(stats->sStats[0].SYSTEM_STATE_RUNNING_CLOCKS), /* event to be monitored */
	     //*(stats->sStats[0].SYSTEM_STATE_RUNNING_CLOCKS), /* event to be monitored */
	     //&systemStateRunningClocks, /* event to be monitored */
	     SYSTEM_STATE_RUNNING_CLOCKS,
	     host,			/* hostE host	*/
	     target 			/*targetE target*/
	     )
	    )
	  
	  //for (ahc=0;ahc<ACTIVE_HC;ahc++)
	  {
	    int ahc=0;
	    /* ######################################
	     * Step_1 :
	     * Report counter0
	     ######################################## */
	    if (err=
		vtApiRdInstrumentation(
				       galaxyConfig,	/* galaxyConfigT galaxyConfig */
				       0 ,				/* instrPeriphId */
				       0, 				/* int system 	*/
				       0, 				/* int context 	*/
				       0,				/* int hContext */
				       ahc * 16 + 0,				/* int counter */
				       (long unsigned int *)&(stats->hcStats[0][0][ahc].HC_COMMITED_INSTRUCTIONS), /* event to be monitored */
				       HC_COMMITED_INSTRUCTIONS,
				       host,			/* hostE host	*/
				       target 			/*targetE target*/))
	      exit (FAILURE);
	    /* ######################################
	     * Step_1 :
	     * Report counter1
	     ######################################## */
	    if (err=
   		vtApiRdInstrumentation(
				       galaxyConfig,	/* galaxyConfigT galaxyConfig */
				       0 ,				/* instrPeriphId */
				       0, 				/* int system 	*/
				       0, 				/* int context 	*/
				       0,				/* int hContext */
				       ahc * 16 + 1,				/* int counter */
				       (long unsigned int *)&(stats->hcStats[0][0][ahc].HC_STATE_RUNNING_CLOCKS), /* event to be monitored */
				       HC_STATE_RUNNING_CLOCKS,
				       host,			/* hostE host	*/
				       target 			/*targetE target*/))
	      exit (FAILURE);
	    /* ######################################
	     * Step_1 :
	     * Report counter2
	     ######################################## */
	    if (err=
   		vtApiRdInstrumentation(
				       galaxyConfig,	/* galaxyConfigT galaxyConfig */
				       0 ,				/* instrPeriphId */
				       0, 				/* int system 	*/
				       0, 				/* int context 	*/
				       0,				/* int hContext */
				       ahc * 16 + 2,				/* int counter */
				       (long unsigned int *)&(stats->hcStats[0][0][ahc].HC_PIPE_RESTART_BRANCH), /* event to be monitored */
				       HC_PIPE_RESTART_BRANCH,
				       host,			/* hostE host	*/
				       target 			/*targetE target*/))
	      exit (FAILURE);
	    /* ######################################
	     * Step_1 :
	     * Report counter3
	     ######################################## */
	    if (err=
   		vtApiRdInstrumentation(
				       galaxyConfig,	/* galaxyConfigT galaxyConfig */
				       0 ,				/* instrPeriphId */
				       0, 				/* int system 	*/
				       0, 				/* int context 	*/
				       0,				/* int hContext */
				       ahc * 16 + 3,				/* int counter */
				       (long unsigned int *)&(stats->hcStats[0][0][ahc].HC_PIPE_RESTART_CALL),
				       HC_PIPE_RESTART_CALL,
				       host,			/* hostE host	*/
				       target 			/*targetE target*/))
	      exit (FAILURE);

	    /* ######################################
	     * Step_1 :
	     * Report counter4
	     ######################################## */
	    if (err=
   		vtApiRdInstrumentation(
				       galaxyConfig,	/* galaxyConfigT galaxyConfig */
				       0 ,				/* instrPeriphId */
				       0, 				/* int system 	*/
				       0, 				/* int context 	*/
				       0,				/* int hContext */
				       ahc * 16 + 4,				/* int counter */
				       (long unsigned int *)&(stats->hcStats[0][0][ahc].HC_PIPE_RESTART_RET),
				       HC_PIPE_RESTART_RET, 
				       host,			/* hostE host	*/
				       target 			/*targetE target*/))
	      exit (FAILURE);

	    /* ######################################
	     * Step_1 :
	     * Report counter5
	     ######################################## */
	    if (err=
   		vtApiRdInstrumentation(
				       galaxyConfig,	/* galaxyConfigT galaxyConfig */
				       0 ,				/* instrPeriphId */
				       0, 				/* int system 	*/
				       0, 				/* int context 	*/
				       0,				/* int hContext */
				       ahc * 16 + 5,				/* int counter */
				       (long unsigned int *)&(stats->hcStats[0][0][ahc].HC_IFEI_STALL),
				       HC_IFEI_STALL, 
				       host,			/* hostE host	*/
				       target 			/*targetE target*/))
	      exit (FAILURE);

	    /* ######################################
	     * Step_1 :
	     * Report counter6
	     ######################################## */
	    if (err=
   		vtApiRdInstrumentation(
				       galaxyConfig,	/* galaxyConfigT galaxyConfig */
				       0 ,				/* instrPeriphId */
				       0, 				/* int system 	*/
				       0, 				/* int context 	*/
				       0,				/* int hContext */
				       ahc * 16 + 6,				/* int counter */
				       (long unsigned int *)&(stats->hcStats[0][0][ahc].HC_IFE_NO_FETCH),
				       HC_IFE_NO_FETCH,  
				       host,			/* hostE host	*/
				       target 			/*targetE target*/))
	      exit (FAILURE);

	    /* ######################################
	     * Step_1 :
	     * Report counter7
	     ######################################## */
	    if (err=
   		vtApiRdInstrumentation(
				       galaxyConfig,	/* galaxyConfigT galaxyConfig */
				       0 ,				/* instrPeriphId */
				       0, 				/* int system 	*/
				       0, 				/* int context 	*/
				       0,				/* int hContext */
				       ahc * 16 + 7,				/* int counter */
				       (long unsigned int *)&(stats->hcStats[0][0][ahc].HC_LSU_HOLD),
				       HC_LSU_HOLD, 
				       host,			/* hostE host	*/
				       target 			/*targetE target*/))
	      exit (FAILURE);

	    /* ######################################
	     * Step_1 :
	     * Report counter8
	     ######################################## */
	    if (err=
   		vtApiRdInstrumentation(
				       galaxyConfig,	/* galaxyConfigT galaxyConfig */
				       0 ,				/* instrPeriphId */
				       0, 				/* int system 	*/
				       0, 				/* int context 	*/
				       0,				/* int hContext */
				       ahc * 16 + 8,				/* int counter */
				       (long unsigned int *)&(stats->hcStats[0][0][ahc].HC_SCRBD_S_GPR_STALL),
				       HC_SCRBD_S_GPR_STALL,
				       host,			/* hostE host	*/
				       target 			/*targetE target*/))
	      exit (FAILURE);

	    /* ######################################
	     * Step_1 :
	     * Report counter9
	     ######################################## */
	    if (err=
   		vtApiRdInstrumentation(
				       galaxyConfig,	/* galaxyConfigT galaxyConfig */
				       0 ,				/* instrPeriphId */
				       0, 				/* int system 	*/
				       0, 				/* int context 	*/
				       0,				/* int hContext */
				       ahc * 16 + 9,				/* int counter */
				       (long unsigned int *)&(stats->hcStats[0][0][ahc].HC_SCRBD_S_BR_STALL),
				       HC_SCRBD_S_BR_STALL, 
				       host,			/* hostE host	*/
				       target 			/*targetE target*/))
	      exit (FAILURE);

	    /* ######################################
	     * Step_1 :
	     * Report counter10
	     ######################################## */
	    if (err=
   		vtApiRdInstrumentation(
				       galaxyConfig,	/* galaxyConfigT galaxyConfig */
				       0 ,				/* instrPeriphId */
				       0, 				/* int system 	*/
				       0, 				/* int context 	*/
				       0,				/* int hContext */
				       ahc * 16 + 10,				/* int counter */
				       (long unsigned int *)&(stats->hcStats[0][0][ahc].HC_SCRBD_S_LR_STALL),
				       HC_SCRBD_S_LR_STALL,  
				       host,			/* hostE host	*/
				       target 			/*targetE target*/))

	      /* ######################################
	       * Step_1 :
	       * Report counter11
	       ######################################## */
	      if (err=
		  vtApiRdInstrumentation(
					 galaxyConfig,	/* galaxyConfigT galaxyConfig */
					 0 ,				/* instrPeriphId */
					 0, 				/* int system 	*/
					 0, 				/* int context 	*/
					 0,				/* int hContext */
					 ahc * 16 + 11,				/* int counter */
					 (long unsigned int *)&(stats->hcStats[0][0][ahc].HC_SCRBD_STALL),
					 HC_SCRBD_STALL,
					 host,			/* hostE host	*/
					 target 			/*targetE target*/))
		exit (FAILURE);

	    /* ######################################
	     * Step_1 :
	     * Report counter12
	     ######################################## */
	    if (err=
   		vtApiRdInstrumentation(
				       galaxyConfig,	/* galaxyConfigT galaxyConfig */
				       0 ,				/* instrPeriphId */
				       0, 				/* int system 	*/
				       0, 				/* int context 	*/
				       0,				/* int hContext */
				       ahc * 16 + 12,				/* int counter */
				       (long unsigned int *)&(stats->cStats[0][0].C_CLOCKS),
				       C_CLOCKS,
				       host,			/* hostE host	*/
				       target 			/*targetE target*/))
	      exit (FAILURE);

	    /* ######################################
	     * Step_1 :
	     * Report counter13
	     ######################################## */
	    if (err=
   		vtApiRdInstrumentation(
				       galaxyConfig,	/* galaxyConfigT galaxyConfig */
				       0 ,				/* instrPeriphId */
				       0, 				/* int system 	*/
				       0, 				/* int context 	*/
				       0,				/* int hContext */
				       ahc * 16 + 12,				/* int counter */
				       &hcSystemStateRunningOffset, /* event to be monitored */
				       HC_SYSTEM_STATE_RUNNING_OFFSET,
				       host,			/* hostE host	*/
				       target 			/*targetE target*/))
	      exit (FAILURE);



	  }
      }
      }
}

/************************************************************************************************
 * updateState: Updates the state of the addressed S.C.HC.TARGET with the new value
 ************************************************************************************************/
int updateState (unsigned int system ,
		 unsigned int context ,
		 unsigned int hypercontext ,
		 targetE target,
		 vtCtrlStateE state)
{

}

/************************************************************************************************
 * Implements the vthread_create in C, making use of the vtcu

pthread_create(3)        BSD Library Functions Manual        pthread_create(3)



NAME

     pthread_create -- create a new thread



SYNOPSIS

     #include <pthread.h>

     int
     pthread_create(pthread_t *restrict thread,
         const pthread_attr_t *restrict attr, void *(*start_routine)(void *),
         void *restrict arg);



DESCRIPTION

     The pthread_create() function is used to create a new thread, with
     attributes specified by attr, within a process.  If attr is NULL, the
     default attributes are used.  If the attributes specified by attr are
     modified later, the thread's attributes are not affected.  Upon success-
     ful completion, pthread_create() will store the ID of the created thread
     in the location specified by thread.

     Upon its creation, the thread executes start_routine, with arg as its
     sole argument.  If start_routine returns, the effect is as if there was
     an implicit call to pthread_exit(), using the return value of
     start_routine as the exit status.  Note that the thread in which main()
     was originally invoked differs from this.  When it returns from main(),
     the effect is as if there was an implicit call to exit(), using the
     return value of main() as the exit status.

     The signal state of the new thread is initialized as:

           o   The signal mask is inherited from the creating thread.

           o   The set of signals pending for the new thread is empty.



RETURN VALUES

     If successful,  the pthread_create() function will return zero.  Other-
     wise, an error number will be returned to indicate the error.



ERRORS

     pthread_create() will fail if:

     [EAGAIN]           The system lacked the necessary resources to create
                        another thread, or the system-imposed limit on the
                        total number of threads in a process
                        [PTHREAD_THREADS_MAX] would be exceeded.

     [EINVAL]           The value specified by attr is invalid.



Asm2(0x0) int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);
Rd1=0 (successful creation of thread)
Rd2= vthread handle
RETURN VALUE
If successful, the pthread_create() function returns zero.
Otherwise, an error number is returned to indicate the error.
ERRORS
[EAGAIN]
The system lacked the necessary resources to
create another thread, or the system-imposed limit
on the total number of threads in a process
PTHREAD_THREADS_MAX would be exceeded.
[EINVAL]
The value specified by attr is invalid.
[EPERM]
The caller does not have appropriate permission
to set the required scheduling parameters or
scheduling policy.
************************************************************************************************/
int vthread_create_local(int *thread,
			 int *attr,
			 int * start_routine,
			 int * arg)
{


  return(0);
}

/************************************************************************************************/
  int vthread_create_remote(int *thread,
			    int *attr,
			    int * start_routine,
			    int * arg)
{


  return(0);
}

/************************************************************************************************/
/*
  pthread_join(3)          BSD Library Functions Manual          pthread_join(3)



  NAME

  pthread_join -- wait for thread termination



  SYNOPSIS

  #include <pthread.h>

  int
  pthread_join(pthread_t thread, void **value_ptr);



  DESCRIPTION

  The pthread_join() function suspends execution of the calling thread
  until the target thread terminates, unless the target thread has already
  terminated.

  On return from a successful pthread_join() call with a non-NULL value_ptr
  argument, the value passed to pthread_exit() by the terminating thread is
  stored in the location referenced by value_ptr.  When a pthread_join()
  returns successfully, the target thread has been terminated.  The results
  of multiple simultaneous calls to pthread_join(), specifying the same
  target thread, are undefined.  If the thread calling pthread_join() is
  cancelled, the target thread is not detached.



  RETURN VALUES

  If successful,  the pthread_join() function will return zero.  Otherwise,
  an error number will be returned to indicate the error.



  ERRORS

  pthread_join() will fail if:

  [EDEADLK]          A deadlock was detected or the value of thread speci-
  fies the calling thread.

  [EINVAL]           The implementation has detected that the value speci-
  fied by thread does not refer to a joinable thread.

  [ESRCH]            No thread could be found corresponding to that speci-
  fied by the given thread ID, thread.



*/
int vthread_join(int *thread,
		 int **value_ptr)
{


  return(0);
}

/************************************************************************************************/
/*
  pthread_equal(3)         BSD Library Functions Manual         pthread_equal(3)



  NAME

  pthread_equal -- compare thread IDs



  SYNOPSIS

  #include <pthread.h>

  int
  pthread_equal(pthread_t t1, pthread_t t2);



  DESCRIPTION

  The pthread_equal() function compares the thread IDs t1 and t2.



  RETURN VALUES

  The pthread_equal() function will return non-zero if the thread IDs t1
  and t2 correspond to the same thread. Otherwise, it will return zero.



  ERRORS

  None.



*/
int vthread_equal(int *t1,
		  int *t2)
{


  return(0);
}
/************************************************************************************************/
/*
  pthread_exit(3)          BSD Library Functions Manual          pthread_exit(3)



  NAME

  pthread_exit -- terminate the calling thread



  SYNOPSIS

  #include <pthread.h>

  void
  pthread_exit(void *value_ptr);



  DESCRIPTION

  The pthread_exit() function terminates the calling thread and makes the
  value value_ptr available to any successful join with the terminating
  thread.  Any cancellation cleanup handlers that have been pushed and are
  not yet popped are popped in the reverse order that they were pushed and
  then executed.  After all cancellation handlers have been executed, if
  the thread has any thread-specific data, appropriate destructor functions
  are called in an unspecified order.  Thread termination does not release
  any application visible process resources, including, but not limited to,
  mutexes and file descriptors, nor does it perform any process level
  cleanup actions, including, but not limited to, calling atexit() routines
  that may exist.

  An implicit call to pthread_exit() is made when a thread other than the
  thread in which main() was first invoked returns from the start routine
  that was used to create it. The function's return value serves as the
  thread's exit status.

  The behavior of pthread_exit() is undefined if called from a cancellation
  handler or destructor function that was invoked as the result of an
  implicit or explicit call to pthread_exit().

  After a thread has terminated, the result of access to local (auto) vari-
  ables of the thread is undefined.  Thus, references to local variables of
  the exiting thread should not be used for the pthread_exit() value_ptr
  parameter value.

  The process will exit with an exit status of 0 after the last thread has
  been terminated.  The behavior is as if the implementation called exit()
  with a zero argument at thread termination time.



  RETURN VALUES

  The pthread_exit() function cannot return to its caller.



  ERRORS

  None.



*/
int vthread_exit(int *value_ptr)
{


  return(0);
}
/************************************************************************************************/
/*
  pthread_self(3)          BSD Library Functions Manual          pthread_self(3)



  NAME

  pthread_self -- get the calling thread's ID



  SYNOPSIS

  #include <pthread.h>

  pthread_t
  pthread_self(void);



  DESCRIPTION

  The pthread_self() function returns the thread ID of the calling thread.



  RETURN VALUES

  The pthread_self() function returns the thread ID of the calling thread.



  ERRORS

  None.



*/
int vthread_self(void)
{


  return(0);
}

/************************************************************************************************
pthread_mutex_init(3)    BSD Library Functions Manual    pthread_mutex_init(3)



NAME

     pthread_mutex_init -- create a mutex



SYNOPSIS

     #include <pthread.h>

     int
     pthread_mutex_init(pthread_mutex_t *restrict mutex,
         const pthread_mutexattr_t *restrict attr);



DESCRIPTION

     The pthread_mutex_init() function creates a new mutex, with attributes
     specified with attr.  If attr is NULL, the default attributes are used.



RETURN VALUES

     If successful, pthread_mutex_init() will return zero and put the new
     mutex id into mutex.  Otherwise, an error number will be returned to
     indicate the error.



ERRORS

     pthread_mutex_init() will fail if:

     [EAGAIN]           The system temporarily lacks the resources to create
                        another mutex.

     [EINVAL]           The value specified by attr is invalid.

     [ENOMEM]           The process cannot allocate enough memory to create
                        another mutex.


*/
int vthread_mutex_init(int *thread,
		       int *attr,
		       int * start_routine,
		       int * arg)
{


  return(0);
}
/************************************************************************************************
 *pthread_mutex_lock(3)    BSD Library Functions Manual    pthread_mutex_lock(3)



NAME

     pthread_mutex_lock -- lock a mutex



SYNOPSIS

     #include <pthread.h>

     int
     pthread_mutex_lock(pthread_mutex_t *mutex);



DESCRIPTION

     The pthread_mutex_lock() function locks mutex.  If the mutex is already
     locked, the calling thread will block until the mutex becomes available.



RETURN VALUES

     If successful, pthread_mutex_lock() will return zero.  Otherwise, an
     error number will be returned to indicate the error.



ERRORS

     pthread_mutex_lock() will fail if:

     [EDEADLK]          A deadlock would occur if the thread blocked waiting
                        for mutex.

     [EINVAL]           The value specified by mutex is invalid.



************************************************************************************************/
int vthread_mutex_lock(int *thread,
		       int *attr,
		       int * start_routine,
		       int * arg)
{


  return(0);
}
/************************************************************************************************/
/*
  pthread_mutex_trylock(3) BSD Library Functions Manual pthread_mutex_trylock(3)



  NAME

  pthread_mutex_trylock -- attempt to lock a mutex without blocking



  SYNOPSIS

  #include <pthread.h>

  int
  pthread_mutex_trylock(pthread_mutex_t *mutex);



  DESCRIPTION

  The pthread_mutex_trylock() function locks mutex.  If the mutex is
  already locked, pthread_mutex_trylock() will not block waiting for the
  mutex, but will return an error condition.



  RETURN VALUES

  If successful, pthread_mutex_trylock() will return zero.  Otherwise, an
  error number will be returned to indicate the error.



  ERRORS

  pthread_mutex_trylock() will fail if:

  [EBUSY]            Mutex is already locked.

  [EINVAL]           The value specified by mutex is invalid.



*/
int vthread_mutex_trylock(int *thread,
			  int *attr,
			  int * start_routine,
			  int * arg)
{


  return(0);
}
/************************************************************************************************/
/*
  pthread_mutex_destroy(3) BSD Library Functions Manual pthread_mutex_destroy(3)



  NAME

  pthread_mutex_destroy -- free resources allocated for a mutex



  SYNOPSIS

  #include <pthread.h>

  int
  pthread_mutex_destroy(pthread_mutex_t *mutex);



  DESCRIPTION

  The pthread_mutex_destroy() function frees the resources allocated for
  mutex.



  RETURN VALUES

  If successful, pthread_mutex_destroy() will return zero.  Otherwise, an
  error number will be returned to indicate the error.



  ERRORS

  pthread_mutex_destroy() will fail if:

  [EBUSY]            Mutex is locked by a thread.

  [EINVAL]           The value specified by mutex is invalid.



*/
int vthread_mutex_destroy(int *thread,
			  int *attr,
			  int * start_routine,
			  int * arg)
{


  return(0);
}
/************************************************************************************************/
/*
  pthread_mutex_unlock(3)  BSD Library Functions Manual  pthread_mutex_unlock(3)



  NAME

  pthread_mutex_unlock -- unlock a mutex



  SYNOPSIS

  #include <pthread.h>

  int
  pthread_mutex_unlock(pthread_mutex_t *mutex);



  DESCRIPTION

  If the current thread holds the lock on mutex, then the
  pthread_mutex_unlock() function unlocks mutex.

  Calling pthread_mutex_unlock() with a mutex that the calling thread does
  not hold will result in undefined behavior.



  RETURN VALUES

  If successful, pthread_mutex_unlock() will return zero.  Otherwise, an
  error number will be returned to indicate the error.



  ERRORS

  pthread_mutex_unlock() will fail if:

  [EINVAL]           The value specified by mutex is invalid.

  [EPERM]            The current thread does not hold a lock on mutex.



*/
int vthread_mutex_unlock(int *thread,
			 int *attr,
			 int * start_routine,
			 int * arg)
{


  return(0);
}

/************************************************************************************************/
/*
  pthread_cond_init(3)     BSD Library Functions Manual     pthread_cond_init(3)



  NAME

  pthread_cond_init -- create a condition variable



  SYNOPSIS

  #include <pthread.h>

  int
  pthread_cond_init(pthread_cond_t *restrict cond,
  const pthread_condattr_t *restrict attr);



  DESCRIPTION

  The pthread_cond_init() function creates a new condition variable, with
  attributes specified with attr.  If attr is NULL, the default attributes
  are used.



  RETURN VALUES

  If successful, the pthread_cond_init() function will return zero and put
  the new condition variable id into cond.  Otherwise, an error number will
  be returned to indicate the error.



  ERRORS

  pthread_cond_init() will fail if:

  [EAGAIN]           The system temporarily lacks the resources to create
  another condition variable.

  [EINVAL]           The value specified by attr is invalid.

  [ENOMEM]           The process cannot allocate enough memory to create
  another condition variable.



*/
int vthread_cond_init(int *thread,
		      int *attr,
		      int * start_routine,
		      int * arg)
{


  return(0);
}
/************************************************************************************************/
/*
  pthread_cond_destroy(3)  BSD Library Functions Manual  pthread_cond_destroy(3)



  NAME

  pthread_cond_destroy -- destroy a condition variable



  SYNOPSIS

  #include <pthread.h>

  int
  pthread_cond_destroy(pthread_cond_t *cond);



  DESCRIPTION

  The pthread_cond_destroy() function frees the resources allocated by the
  condition variable cond.



  RETURN VALUES

  If successful, the pthread_cond_destroy() function will return zero.
  Otherwise, an error number will be returned to indicate the error.



  ERRORS

  pthread_cond_destroy() will fail if:

  [EBUSY]            The variable cond is locked by another thread.

  [EINVAL]           The value specified by cond is invalid.



*/
int vthread_cond_destroy(int *thread,
			 int *attr,
			 int * start_routine,
			 int * arg)
{


  return(0);
}
/************************************************************************************************/
/*
  pthread_cond_signal(3)   BSD Library Functions Manual   pthread_cond_signal(3)



  NAME

  pthread_cond_signal -- unblock a thread waiting for a condition variable



  SYNOPSIS

  #include <pthread.h>

  int
  pthread_cond_signal(pthread_cond_t *cond);



  DESCRIPTION

  The pthread_cond_signal() function unblocks one thread waiting for the
  condition variable cond.



  RETURN VALUES

  If successful, the pthread_cond_signal() function will return zero.  Oth-
  erwise, an error number will be returned to indicate the error.



  ERRORS

  pthread_cond_signal() will fail if:

  [EINVAL]           The value specified by cond is invalid.



*/
int vthread_cond_signal(int *thread,
			int *attr,
			int * start_routine,
			int * arg)
{


  return(0);
}
/************************************************************************************************/
/*
  PTHREAD_COND_BROADCAS... BSD Library Functions Manual PTHREAD_COND_BROADCAS...



  NAME

  pthread_cond_broadcast -- unblock all threads waiting for a condition
  variable



  SYNOPSIS

  #include <pthread.h>

  int
  pthread_cond_broadcast(pthread_cond_t *cond);



  DESCRIPTION

  The pthread_cond_broadcast() function unblocks all threads that are wait-
  ing for the condition variable cond.



  RETURN VALUES

  If successful, the pthread_cond_broadcast() function will return zero.
  Otherwise, an error number will be returned to indicate the error.



  ERRORS

  pthread_cond_broadcast() will fail if:

  [EINVAL]           The value specified by cond is invalid.



*/
int vthread_cond_broardcast(int *thread,
			    int *attr,
			    int * start_routine,
			    int * arg)
{


  return(0);
}
/************************************************************************************************/
/*
  pthread_cond_wait(3)     BSD Library Functions Manual     pthread_cond_wait(3)



  NAME

  pthread_cond_wait -- wait on a condition variable



  SYNOPSIS

  #include <pthread.h>

  int
  pthread_cond_wait(pthread_cond_t *restrict cond,
  pthread_mutex_t *restrict mutex);



  DESCRIPTION

  The pthread_cond_wait() function atomically unlocks the mutex argument
  and waits on the cond argument. Before returning control to the calling
  function, pthread_cond_wait() re-acquires the mutex.


  RETURN VALUES

  If successful, the pthread_cond_wait() function will return zero.  Other-
  wise, an error number will be returned to indicate the error.



  ERRORS

  pthread_cond_wait() will fail if:

  [EINVAL]           The value specified by cond or the value specified by
  mutex is invalid.



*/
int vthread_cond_wait(int *thread,
		      int *attr,
		      int * start_routine,
		      int * arg)
{


  return(0);
}


/************************************************************************************************
 * matchSyll: Checks the syll against the input pattern; Return 1 if true
 ************************************************************************************************/
int matchSyll (int syll , char * PATTERN)
{
  printf ("Matching against %s\n", PATTERN);
}

/************************************************************************************************
 * postClock
 * Called after insizzleClock. Scans all the instructions that have executed for:
 * HALT, vthread_create_local, vthread_create_remote, vthread_mutex_init, vthread_mutex_trylock,
 * vthread_mutex_lock, vthread_mutex_unlock, vthread_mutex_destroy,VTHREAD_JOIN, VTHREAD_EXIT,
 * VTHREAD_SELF, VTHREAD_EQUAL, VTHREAD_COND_INIT, VTHREAD_COND_DESTROY, VTHREAD_COND_SIGNAL,
 * VTHREAD_COND_BROADCAST, VTHREAD_COND_WAIT, VTHREAD_COND_TIMEDWAIT
 ************************************************************************************************/
int postClock(
	      galaxyConfigT *galaxyConfig,
	      //unsigned int system ,
	      //unsigned int context ,
	      //unsigned int hypercontext,
	      gTracePacketT gTracePacket)
{
  int sys, contexts, hcontexts, syllable;
  for (sys=0 ; sys < galaxyConfig->ctrlState.GALAXY_CONFIG & GALAXY_CONFIG_REG_SYSTEMS_MASK; sys++) {
    for (contexts=0; contexts<(galaxyConfig->ctrlState.SYSTEM_CONFIG[sys] & SYSTEM_CONFIG_REG_CONTEXTS_MASK); contexts++) {
      for (hcontexts=0 ; hcontexts < (((galaxyConfig->ctrlState.CONTEXT_CONFIG[sys][contexts]) >>CONTEXT_CONFIG_REG_HCONTEXTS_SHIFT) & CONTEXT_CONFIG_REG_HCONTEXTS_MASK) ; hcontexts++) {

	// For all HCs not in DEBUG state
	if (gTracePacket[sys][contexts][hcontexts].vt_ctrl != DEBUG)
	  {
	    // Now, scan opcode for any of the captured opcodes
	    /************ VTHREAD_CREATE_LOCAL *************/
	    for (syllable=0;syllable<MASTERCFG_CL_ISSUE_WIDTH; syllable++) {
	      if (matchSyll(gTracePacket[sys][contexts][hcontexts].bundle[syllable].syll , PATTERN_HALT)) updateState(sys,contexts,hcontexts,INSIZZLE, (vtCtrlStateE)DEBUG);
	      if (matchSyll(gTracePacket[sys][contexts][hcontexts].bundle[syllable].syll , PATTERN_VTHREAD_CREATE_LOCAL)) {
		int val;
		//val = vthread_create_local();
		// update registers here
		// update vt_ctrl
		updateState(sys,contexts,hcontexts,INSIZZLE, (vtCtrlStateE)DEBUG);
	      }
	    }
	    /************ VTHREAD_CREATE_REMOTE *************/
	    for (syllable=0;syllable<MASTERCFG_CL_ISSUE_WIDTH; syllable++) {
	      if (matchSyll(gTracePacket[sys][contexts][hcontexts].bundle[syllable].syll , PATTERN_VTHREAD_CREATE_REMOTE)) {
		int val;
		//val = vthread_create_local();
		// update registers here
		// update vt_ctrl
		updateState(sys,contexts,hcontexts,INSIZZLE, (vtCtrlStateE)DEBUG);
	      }
	    }
	    /************ VTHREAD_JOIN *************/
	    for (syllable=0;syllable<MASTERCFG_CL_ISSUE_WIDTH; syllable++) {
	      if (matchSyll(gTracePacket[sys][contexts][hcontexts].bundle[syllable].syll , PATTERN_VTHREAD_JOIN)) {
		int val;
		//val = vthread_create_local();
		// update registers here
		// update vt_ctrl
		updateState(sys,contexts,hcontexts,INSIZZLE, (vtCtrlStateE)DEBUG);
	      }
	    }
	    /************ VTHREAD_EXIT *************/
	    for (syllable=0;syllable<MASTERCFG_CL_ISSUE_WIDTH; syllable++) {
	      if (matchSyll(gTracePacket[sys][contexts][hcontexts].bundle[syllable].syll , PATTERN_VTHREAD_EXIT)) {
		int val;
		//val = vthread_create_local();
		// update registers here
		// update vt_ctrl
		updateState(sys,contexts,hcontexts,INSIZZLE, (vtCtrlStateE)DEBUG);
	      }
	    }
	    /************ VTHREAD_SELF *************/
	    for (syllable=0;syllable<MASTERCFG_CL_ISSUE_WIDTH; syllable++) {
	      if (matchSyll(gTracePacket[sys][contexts][hcontexts].bundle[syllable].syll , PATTERN_VTHREAD_SELF)) {
		int val;
		//val = vthread_create_local();
		// update registers here
		// update vt_ctrl
		updateState(sys,contexts,hcontexts,INSIZZLE, (vtCtrlStateE)DEBUG);
	      }
	    }
	    /************ VTHREAD_EQUAL *************/
	    for (syllable=0;syllable<MASTERCFG_CL_ISSUE_WIDTH; syllable++) {
	      if (matchSyll(gTracePacket[sys][contexts][hcontexts].bundle[syllable].syll , PATTERN_VTHREAD_EQUAL)) {
		int val;
		//val = vthread_create_local();
		// update registers here
		// update vt_ctrl
		updateState(sys,contexts,hcontexts,INSIZZLE, (vtCtrlStateE)DEBUG);
	      }
	    }
	    /************ VTHREAD_MUTEX_INIT *************/
	    for (syllable=0;syllable<MASTERCFG_CL_ISSUE_WIDTH; syllable++) {
	      if (matchSyll(gTracePacket[sys][contexts][hcontexts].bundle[syllable].syll , PATTERN_VTHREAD_MUTEX_INIT)) {
		int val;
		//val = vthread_create_local();
		// update registers here
		// update vt_ctrl
		updateState(sys,contexts,hcontexts,INSIZZLE, (vtCtrlStateE)DEBUG);
	      }
	    }
	    /************ VTHREAD_MUTEX_INIT *************/
	    for (syllable=0;syllable<MASTERCFG_CL_ISSUE_WIDTH; syllable++) {
	      if (matchSyll(gTracePacket[sys][contexts][hcontexts].bundle[syllable].syll , PATTERN_VTHREAD_MUTEX_INIT)) {
		int val;
		//val = vthread_create_local();
		// update registers here
		// update vt_ctrl
		updateState(sys,contexts,hcontexts,INSIZZLE, (vtCtrlStateE)DEBUG);
	      }
	    }
	    /************ VTHREAD_MUTEX_LOCK *************/
	    for (syllable=0;syllable<MASTERCFG_CL_ISSUE_WIDTH; syllable++) {
	      if (matchSyll(gTracePacket[sys][contexts][hcontexts].bundle[syllable].syll , PATTERN_VTHREAD_MUTEX_TRYLOCK)) {
		int val;
		//val = vthread_create_local();
		// update registers here
		// update vt_ctrl
		updateState(sys,contexts,hcontexts,INSIZZLE, (vtCtrlStateE)DEBUG);
	      }
	    }
	    /************ VTHREAD_MUTEX_LOCK *************/
	    for (syllable=0;syllable<MASTERCFG_CL_ISSUE_WIDTH; syllable++) {
	      if (matchSyll(gTracePacket[sys][contexts][hcontexts].bundle[syllable].syll , PATTERN_VTHREAD_MUTEX_LOCK)) {
		int val;
		//val = vthread_create_local();
		// update registers here
		// update vt_ctrl
		updateState(sys,contexts,hcontexts,INSIZZLE, (vtCtrlStateE)DEBUG);
	      }
	    }
	    /************ VTHREAD_MUTEX_UNLOCK *************/
	    for (syllable=0;syllable<MASTERCFG_CL_ISSUE_WIDTH; syllable++) {
	      if (matchSyll(gTracePacket[sys][contexts][hcontexts].bundle[syllable].syll , PATTERN_VTHREAD_MUTEX_UNLOCK)) {
		int val;
		//val = vthread_create_local();
		// update registers here
		// update vt_ctrl
		updateState(sys,contexts,hcontexts,INSIZZLE, (vtCtrlStateE)DEBUG);
	      }
	    }
	    /************ VTHREAD_MUTEX_DESTROY *************/
	    for (syllable=0;syllable<MASTERCFG_CL_ISSUE_WIDTH; syllable++) {
	      if (matchSyll(gTracePacket[sys][contexts][hcontexts].bundle[syllable].syll , PATTERN_VTHREAD_MUTEX_DESTROY)) {
		int val;
		//val = vthread_create_local();
		// update registers here
		// update vt_ctrl
		updateState(sys,contexts,hcontexts,INSIZZLE, (vtCtrlStateE)DEBUG);
	      }
	    }
	    /************ VTHREAD_COND_INIT *************/
	    for (syllable=0;syllable<MASTERCFG_CL_ISSUE_WIDTH; syllable++) {
	      if (matchSyll(gTracePacket[sys][contexts][hcontexts].bundle[syllable].syll , PATTERN_VTHREAD_COND_INIT)) {
		int val;
		//val = vthread_create_local();
		// update registers here
		// update vt_ctrl
		updateState(sys,contexts,hcontexts,INSIZZLE, (vtCtrlStateE)DEBUG);
	      }
	    }
	    /************ VTHREAD_COND_DESTROY *************/
	    for (syllable=0;syllable<MASTERCFG_CL_ISSUE_WIDTH; syllable++) {
	      if (matchSyll(gTracePacket[sys][contexts][hcontexts].bundle[syllable].syll , PATTERN_VTHREAD_COND_DESTROY)) {
		int val;
		//val = vthread_create_local();
		// update registers here
		// update vt_ctrl
		updateState(sys,contexts,hcontexts,INSIZZLE, (vtCtrlStateE)DEBUG);
	      }
	    }
	    /************ VTHREAD_COND_SIGNAL *************/
	    for (syllable=0;syllable<MASTERCFG_CL_ISSUE_WIDTH; syllable++) {
	      if (matchSyll(gTracePacket[sys][contexts][hcontexts].bundle[syllable].syll , PATTERN_VTHREAD_COND_SIGNAL)) {
		int val;
		//val = vthread_create_local();
		// update registers here
		// update vt_ctrl
		updateState(sys,contexts,hcontexts,INSIZZLE, (vtCtrlStateE)DEBUG);
	      }
	    }
	    /************ VTHREAD_COND_BROADCAST *************/
	    for (syllable=0;syllable<MASTERCFG_CL_ISSUE_WIDTH; syllable++) {
	      if (matchSyll(gTracePacket[sys][contexts][hcontexts].bundle[syllable].syll , PATTERN_VTHREAD_COND_BROADCAST)) {
		int val;
		//val = vthread_create_local();
		// update registers here
		// update vt_ctrl
		updateState(sys,contexts,hcontexts,INSIZZLE, (vtCtrlStateE)DEBUG);
	      }
	    }
	    /************ VTHREAD_COND_WAIT *************/
	    for (syllable=0;syllable<MASTERCFG_CL_ISSUE_WIDTH; syllable++) {
	      if (matchSyll(gTracePacket[sys][contexts][hcontexts].bundle[syllable].syll , PATTERN_VTHREAD_COND_WAIT)) {
		int val;
		//val = vthread_create_local();
		// update registers here
		// update vt_ctrl
		updateState(sys,contexts,hcontexts,INSIZZLE, (vtCtrlStateE)DEBUG);
	      }
	    }
	  }
      }
    }
  }
}


/************************************************************************************************
 * vtA[iSysDiags: This is the systems diagnostics code. It runs diagnostics on each S.C.HC
 * as follows
 * 1) DMA Test: For all Systems, burst-write a pattern into the TCDRAM and read it back. To make
 *    things more challenging, local buffers are malloced.
 * 2) IRAM Test : If using 
 * 2) State tests
 * 2a) S_GPR Tests : For all S.C.HC, write pattern into S_GPR and read it back
 * 2b) S_PR Tests : For all S.C.HC clear PRs and then, write then and read them back individually
 ************************************************************************************************/
int vtApiSysDiags(globalTargetT globalTarget, galaxyConfigT galaxyConfig1,  galaxyConfigT  galaxyConfig2)
{
  /* ahbdma registers
   *    when "00" => regd := r.srcaddr;
   *    when "01" => regd := r.dstaddr;
   *    when "10" => regd(20 downto 0) := r.enable & r.srcinc & r.dstinc & r.len;  
   *    enable : 1
   *    srcinc : 2 (drives ahbmo.size)
   *    dstinc : 2 ditto
   *    len : 16 bits : Beats
   */
  printf ("Ready to perform vtApiSysDiags\n");
}



/************************************************************************************************
 * vtApiDbgIfDiags : This is the DbgIf  diagnostics code. 
 ************************************************************************************************/
int vtApiDbgIfDiags (targetE target)
{
  int i; // iteration variable
  unsigned int rdback;
  // Test SYSTEM_REG
#if VT_DEBUG
  printf ("vtApiDbgIfDiags\n");
#endif
  // SYSTEM_REG
  printf ("Checking SYSTEM_REG...");
  for (i=0; i <32 ; i++) {
    vtDbgWrIf(SYSTEM_REG , 0x1<<i , target);
    vtDbgRdIf(SYSTEM_REG , &rdback , target);
    if (rdback != 0x1<<i) {
      //printf ("SYSTEM_REG is %d bits wide\n",i);
      break;
    }
    //else printf ("SYSTEM_REG is 32 bits wide\n");
  }
  printf ("SYSTEM_REG is %d bits wide\n",i);
      //  if (i==32) printf ("SYSTEM_REG is 32 bits wide\n");

  // CONTEXT_REG
  printf ("Checking CONTEXT_REG...");
  for (i=0; i <32 ; i++) {
    vtDbgWrIf(CONTEXT_REG , 0x1<<i , target);
    vtDbgRdIf(CONTEXT_REG , &rdback , target);
    if (rdback != 0x1<<i) {
      //printf ("CONTEXT_REG is %d bits wide\n",i);
      break;
    }
    //else printf ("CONTEXT_REG is 32 bits wide\n");
  }  
  printf ("CONTEXT_REG is %d bits wide\n",i);

  // HYPERCONTEXT_REG
  printf ("Checking HYPERCONTEXT_REG...");
  for (i=0; i <32 ; i++) {
    vtDbgWrIf(HYPERCONTEXT_REG , 0x1<<i , target);
    vtDbgRdIf(HYPERCONTEXT_REG , &rdback , target);
    if (rdback != 0x1<<i) {
      //printf ("HYPERCONTEXT_REG is %d bits wide\n",i);
      break;
    }
    //else printf ("HYPERCONTEXT_REG is 32 bits wide\n");
  }
  printf ("HYPERCONTEXT_REG is %d bits wide\n",i);

 // CLUSTER_REG
  printf ("Checking CLUSTER_REG...");
  for (i=0; i <32 ; i++) {
    vtDbgWrIf(CLUSTER_REG , 0x1<<i , target);
    vtDbgRdIf(CLUSTER_REG , &rdback , target);
    if (rdback != 0x1<<i) {
      //printf ("CLUSTER_REG is %d bits wide\n",i);
      break;
    }
    //else printf ("CLUSTER_REG is 32 bits wide\n");
  }
  printf ("CLUSTER_REG is %d bits wide\n",i);


 // ADDR_SPACE_REG
  printf ("Checking ADDR_SPACE_REG...");
  for (i=0; i <32 ; i++) {
    vtDbgWrIf(ADDR_SPACE_REG , 0x1<<i , target);
    vtDbgRdIf(ADDR_SPACE_REG , &rdback , target);
    if (rdback != 0x1<<i) {
      //printf ("ADDR_SPACE_REG is %d bits wide\n",i);
      break;
    }
    //else printf ("ADDR_SPACE_REG is 32 bits wide\n");
  }
  printf ("ADDR_SPACE_REG is %d bits wide\n",i);

 // ADDR_REG
  printf ("Checking ADDR_REG...");
  for (i=0; i <32 ; i++) {
    vtDbgWrIf(ADDR_REG , 0x1<<i , target);
    vtDbgRdIf(ADDR_REG , &rdback , target);
    if (rdback != 0x1<<i) {
      //printf ("ADDR_REG is %d bits wide\n",i);
      break;
    }
    //else printf ("ADDR_REG is 32 bits wide\n");
  }
      printf ("ADDR_REG is %d bits wide\n",i);

 // WRDATA1_REG
  printf ("Checking WRDATA1_REG...");
  for (i=0; i <32 ; i++) {
    vtDbgWrIf(WRDATA1_REG , 0x1<<i , target);
    vtDbgRdIf(WRDATA1_REG , &rdback , target);
    if (rdback != 0x1<<i) {
      //printf ("WRDATA1_REG is %d bits wide\n",i);
      break;
    }
    //else printf ("WRDATA1_REG is 32 bits wide\n");
  }  
  printf ("WRDATA1_REG is %d bits wide\n",i);
 
 // WRDATA2_REG
  printf ("Checking WRDATA2_REG...");
  for (i=0; i <32 ; i++) {
    vtDbgWrIf(WRDATA2_REG , 0x1<<i , target);
    vtDbgRdIf(WRDATA2_REG , &rdback , target);
    if (rdback != 0x1<<i) {
      //printf ("WRDATA2_REG is %d bits wide\n",i);
      break;
    }
    //else printf ("WRDATA2_REG is 32 bits wide\n");
  }  
  printf ("WRDATA2_REG is %d bits wide\n",i);
      
 // RDDATA1_REG
  /*
  for (i=0; i <32 ; i++) {
    vtDbgWrIf(RDDATA1_REG , 0x1<<i , target);
    vtDbgRdIf(RDDATA1_REG , &rdback , target);
    if (rdback != 0x1<<i) {
      printf ("RDDATA1_REG is %d bits wide\n",i);
      break;
    }
  } 

 // RDDATA2_REG
  for (i=0; i <32 ; i++) {
    vtDbgWrIf(RDDATA2_REG , 0x1<<i , target);
    vtDbgRdIf(RDDATA2_REG , &rdback , target);
    if (rdback != 0x1<<i) {
      printf ("RDDATA2_REG is %d bits wide\n",i);
      break;
    }
  } 
  */
 // STACK_OFFS_REG
  printf ("Checking STACK_OFFS_REG...");
  for (i=0; i <32 ; i++) {
    vtDbgWrIf(STACK_OFFS_REG , 0x1<<i , target);
    vtDbgRdIf(STACK_OFFS_REG , &rdback , target);
    if (rdback != 0x1<<i) {
      //printf ("STACK_OFFS_REG is %d bits wide\n",i);
      break;
    }
    //else printf ("STACK_OFFS_REG is 32 bits wide\n");
  } 
  printf ("STACK_OFFS_REG is %d bits wide\n",i);

//  exit(0);

}
