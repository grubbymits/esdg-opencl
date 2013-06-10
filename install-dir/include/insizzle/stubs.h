/************************************************************************************************
 *	stubs.h 
 * VT low-level interface API header file for interfacing to Insizzle
 * (C) 2011 V. Chouliaras
 * Proprietary and Confidential
 * vassilios@vassilios-chouliaras.com
 ************************************************************************************************/

#include "vtapi.h"
#ifndef STUBS_H_
#define STUBS_H_
/* INSIZZLE STUBS
 * This is the minimum interfacing required with insizzle
 */



int insizzleStubInitVtApi (galaxyConfigT * galaxyConfig );
/* This function clocks the galaxy and updates the trace */
int insizzleClock (galaxyConfigT * galaxyConfig , gTracePacketT  *  gTracePacket );

/* Mandatory INSIZZLE exports */
int insizzleWrOneIramLocation (galaxyConfigT *galaxyConfig ,  int iaddr ,  int data);
int insizzleRdOneIramLocation (galaxyConfigT *galaxyConfig ,  int iaddr ,  int *data);
int insizzleWrOneDramLocation (galaxyConfigT *galaxyConfig ,  int daddr ,  int data);
int insizzleRdOneDramLocation (galaxyConfigT *galaxyConfig ,  int daddr ,  int *data);
int insizzleSetCurrent(unsigned int system , unsigned int context ,unsigned int hypercontext , unsigned int cluster);

/************************************************************************************************
 * Reads one S_GPR from the addressed S.C.HC.CL and store in rdata
 ************************************************************************************************/
int insizzleRdOneSGpr( unsigned int sgpr, unsigned int * rdata);
/************************************************************************************************
 * Stores wdata into the addressed S.C.HC.CL
 ************************************************************************************************/
int insizzleWrOneSGpr(unsigned int sgpr , unsigned int  wdata, targetE target);

/************************************************************************************************
 * returns the LR of the addressed S.C.HC
 ************************************************************************************************/
int insizzleRdOneLr(galaxyConfigT *galaxyConfig , unsigned int * rdata);

/************************************************************************************************
 * Perform a read control register operation
 ************************************************************************************************/
int insizzleRdCtrl(galaxyConfigT *galaxyConfig , vtCtrlStateE  *val);

/************************************************************************************************
 * Write the vt_ctrl.STATE only
 ************************************************************************************************/
int insizzleWrCtrl(galaxyConfigT *galaxyConfig , vtCtrlStateE  val);

/************************************************************************************************
 * Write PC
 ************************************************************************************************/
int insizzleWrPC(galaxyConfigT *galaxyConfig , int  val);

#endif /*STUBS_H_*/
 
