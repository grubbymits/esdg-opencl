#include "stubs.h"
#include "vtapi.h"
#include <stdio.h>


/* This stub function returns a valid galaxyConfig. It can be recovered from the xml file

 */
/* external function from Insizzle API
 */
#ifdef X86_INSIZZLE


extern int insizzleAPIClock(galaxyConfigT *, gTracePacketT gTracePacket[][MASTERCFG_CONTEXTS_MAX][MASTERCFG_HYPERCONTEXTS_MAX]);
extern int insizzleAPIWrOneSGpr(unsigned, unsigned);
extern int insizzleAPIRdOneSGpr(unsigned, unsigned *);
extern int insizzleAPIRdOneLr(galaxyConfigT *, unsigned *);
extern int insizzleAPIWrOneLr(galaxyConfigT *, unsigned);
extern int insizzleAPIRdOneBr(unsigned, unsigned *);
extern int insizzleAPIWrOneBr(unsigned, unsigned);
extern int insizzleAPIRdCtrl(galaxyConfigT *, vtCtrlStateE *);
extern int insizzleAPIWrCtrl(galaxyConfigT *, vtCtrlStateE);
extern int insizzleAPIWrOneIramLocation(galaxyConfigT *, unsigned, unsigned);
extern int insizzleAPIRdOneIramLocation(galaxyConfigT *, unsigned, unsigned *);
extern int insizzleAPIWrOneDramLocation(galaxyConfigT *, unsigned, unsigned);
extern int insizzleAPIRdOneDramLocation(galaxyConfigT *, unsigned, unsigned *);
extern int insizzleAPIWrPC(galaxyConfigT *, unsigned);
extern int insizzleAPIRdPC(galaxyConfigT *, unsigned *);

extern void insizzleAPILdIRAM(char *, int);
extern void insizzleAPILdDRAM(char *, int);

extern int insizzleAPISetCurrent(unsigned, unsigned, unsigned, unsigned);
extern int insizzleAPIStubInitVtApi(galaxyConfigT *);

extern int insizzleAPIoutputCounts(void);

#endif
int insizzleStubInitVtApi (galaxyConfigT * galaxyConfig )
{
  // Read the xml config and populate galaxyConfig. If failed, return
  //INSIZZLE_VTAPI_INIT_FAILURE
#ifdef X86_INSIZZLE
  if(!insizzleAPIStubInitVtApi(galaxyConfig))
    return(SUCCESS);
  else
    return(INSIZZLE_VTAPI_INIT_FAILURE);
#else
  printf ("in insizzleStubInitVtApi (X86_INSIZZLE not defined)\n");
  return (SUCCESS);
#endif
}

/* This stub function clocks the simulated galaxy and updates the dynamic trace */
int insizzleClock (galaxyConfigT * galaxyConfig , gTracePacketT  *gTracePacket )
{
  // Read the xml config and populate galaxyConfig. If failed, return
  //INSIZZLE_VTAPI_INIT_FAILURE
#ifdef X86_INSIZZLE
  if(!insizzleAPIClock(galaxyConfig, gTracePacket))
    return (SUCCESS);
  else
    return (FAILURE);
#else
  printf ("in insizzleClock (X86_INSIZZLE not defined)n");
  return (SUCCESS);
#endif
}

/* This stub function loads the DRAM of system, address daddr with the value data.
 *
 */
int insizzleWrOneDramLocation (galaxyConfigT *galaxyConfig , int daddr , int data)
{
#ifdef X86_INSIZZLE
  if(!insizzleAPIWrOneDramLocation(galaxyConfig, daddr, data))
    return (SUCCESS);
  else
    return (FAILURE);
#else
  printf ("insizzleWrOneDramLocation: Writing  daddr 0x%x with value 0x%x (X86_INSIZZLE not defined)\n",daddr,data);
  return (SUCCESS);
#endif
}
/* This function reads from the data RAM of system(address daddr) the value and returns
   it in *data. Error code is returned normally
*/
int insizzleRdOneDramLocation (galaxyConfigT *galaxyConfig , int daddr , int *data)
{
#ifdef X86_INSIZZLE
  if(!insizzleAPIRdOneDramLocation(galaxyConfig, daddr, data))
    return (SUCCESS);
  else
    return (FAILURE);
#else
  printf ("insizzleRdOneDramLocation: Read daddr 0x%x and got value 0x%x (X86_INSIZZLE not defined)\n",daddr,*data);
  return (SUCCESS);
#endif
}

/* This function reads from the IRAM of system.context(address iaddr) the value and returns
   it in *data. Error code is returned normally
*/
int insizzleRdOneIramLocation (galaxyConfigT *galaxyConfig , int iaddr ,  int *data)
{
#ifdef X86_INSIZZLE
  /*
  if(!insizzleAPIRdOneIramLocation(galaxyConfig, iaddr, data))
    return (SUCCESS);
  else
    return (FAILURE);
#else
  */
  printf ("insizzleRdOneIramLocation: Read iaddr 0x%x and got value 0x%x (X86_INSIZZLE not defined)\n",iaddr,*data);
  return (SUCCESS);
#endif
}

/* This stub function loads the DRAM of system, address daddr with the value data.
 *
 */
int insizzleWrOneIramLocation (galaxyConfigT *galaxyConfig , int iaddr , int data)
{
#ifdef X86_INSIZZLE
  if(!insizzleAPIWrOneIramLocation(galaxyConfig, iaddr, data))
    return (SUCCESS);
  else
    return (FAILURE);
#else
  printf ("insizzleWrOneIramLocation: Writing iaddr 0x%x with value 0x%x (X86_INSIZZLE not defined)\n",iaddr,data);
  return (SUCCESS);
#endif
}


/* This function sets the internal S.C.HC.CL registers in insizzle. Subsequent transactions
   use the stored values in those registers to read from/write to state to the addressed
   S.C.HC.CL
*/
int insizzleSetCurrent(unsigned int system , unsigned int context ,unsigned int hypercontext , unsigned int cluster)
{
#ifdef X86_INSIZZLE
  if(!insizzleAPISetCurrent(system, context, hypercontext, cluster))
    return (SUCCESS);
  else
    return (FAILURE);
#else
  printf ("in insizzleSetCurrent (X86_INSIZZLE not defined)\n");
  return (SUCCESS);
#endif
}

int insizzleRdOneSGpr( unsigned int sgpr, unsigned int * rdata)
{
#ifdef X86_INSIZZLE
  if(!insizzleAPIRdOneSGpr(sgpr, rdata))
    return (SUCCESS);
  else
    return (FAILURE);
#else
  printf ("In insizzleRdOneSGpr (X86_INSIZZLE not defined)\n");
  return (SUCCESS);
#endif
}
int insizzleWrOneSGpr(unsigned int sgpr , unsigned int  wdata)
{
#ifdef X86_INSIZZLE
  if(!insizzleAPIWrOneSGpr(sgpr, wdata))
    return (SUCCESS);
  else
    return (FAILURE);
#else
  printf ("In insizzleWrOneSGpr (X86_INSIZZLE not defined)\n");
  return (SUCCESS);
#endif
}


int insizzleRdOneLr(galaxyConfigT *galaxyConfig , unsigned int * rdata)
{
#ifdef X86_INSIZZLE
  if(!insizzleAPIRdOneLr(galaxyConfig, rdata))
    return (SUCCESS);
  else
    return (FAILURE);
#else
  printf ("In insizzleRdOneLr (X86_INSIZZLE not defined)\n");
  return (SUCCESS);
#endif
}
int insizzleRdCtrl(galaxyConfigT *galaxyConfig , vtCtrlStateE  *val)
{
#ifdef X86_INSIZZLE
  if(!insizzleAPIRdCtrl(galaxyConfig, val))
    return (SUCCESS);
  else
    return (FAILURE);
#else
  printf ("In insizzleRdCtrl (X86_INSIZZLE not defined)\n");
  return (SUCCESS);
#endif
}
int insizzleWrCtrl(galaxyConfigT *galaxyConfig , vtCtrlStateE  val)
{
#ifdef X86_INSIZZLE
  if(!insizzleAPIWrCtrl(galaxyConfig, val)) {
    return (SUCCESS);
  }
  else
    return (FAILURE);
#else
  printf ("In insizzleWrCtrl (X86_INSIZZLE not defined)\n");
  return (SUCCESS);
#endif
}

int insizzleWrPC(galaxyConfigT *galaxyConfig , unsigned int  val)
{
#ifdef X86_INSIZZLE
  if(!insizzleAPIWrPC(galaxyConfig, val))
    return (SUCCESS);
  else
    return (FAILURE);
#else
  printf ("In insizzleWrPC (X86_INSIZZLE not defined)\n");
  return (SUCCESS);
#endif
}
