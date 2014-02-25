#ifndef _FUNCTIONS_H
#define _FUNCTIONS_H

#include <time.h>

#include "galaxy.h"
#include "galaxyConfig.h"

#ifdef INSIZZLEAPI
/*#include "galaxyConfig_STATIC.h"*/
#include "vtapi.h"
#endif

/* deepstate defines */
#ifndef INSIZZLEAPI
#define READY                 0
#define RUNNING               1
#define BLOCKED_MUTEX_LOCK    2
#define TERMINATED_ASYNC_HOST 3
#define TERMINATED_ASYNC      4
#define TERMINATED_SYNC       5
#endif

#define PIPELINE_REFILL       3

#define MAX_GALAXIES          1
#define MAX_SYSTEMS           16 /* per galaxy */
#define MAX_CONTEXTS          16 /* per system */
#define MAX_HYPERCONTEXT      16 /* per context */
#define MAX_CLUSTERS          16 /* per context */

#define INSIZZLE_FAIL -1
#define INSIZZLE_SUCCESS 0


extern unsigned char isLLVM;
extern unsigned memAlign;
extern time_t start, end;
extern unsigned char similarIRAM, suppressOOB;
extern unsigned long long cycleCount;
extern unsigned int STACK_SIZE; /* stack size per hypercontext in KB */
extern unsigned int PRINT_OUT; /* switch to output cycle information */
extern unsigned int SINGLE_STEP;
extern unsigned int STAGGER; /* In multicore cpuid mode start context staggered */

/* list of calls with names, pcs and registers required */
typedef struct callsList_t{
  unsigned pc;
  char *name;
  char registers[7];
  struct callsList_t *next;
} callsList_t;

extern callsList_t *callsList;
/* TODO: remember to free all the memory */
int setupCallsList(char *);

typedef struct {
  unsigned op;
  unsigned imm;
  unsigned char immValid;
  unsigned int nextPC;
} instructionPacket;

typedef enum {
  _R_iss,
  _B_iss,
  _L_iss,
  _MEM_iss,
  _NUL_iss
} targetT;

typedef enum {
  GOTO,CALL,ENTRY,RETURN,BRANCH,BRANCHF,LDSB,LDUB,LDSH,LDUH,
  LDHU,LDW,STB,STH,STW,MLSL,MLUL,MLSH,MLUH,MLSLL,
  MLULL,MLSLH,MLULH,MLSHH,MLUHH,ADD,AND,ANDC,ANDL,CMPEQ,
  CMPGES,CMPGEU,CMPGTS,CMPGTU,CMPLES,CMPLEU,CMPLTS,CMPLTU,CMPNE,MAXS,
  MAXU,MINS,MINU,MFB,MFL,MOV,MTL,MTB,MTBF,MPY,
  NANDL,NOP,NORL,ORL,OR,ORC,PFT,SBIT,SBITF,SH1ADD,
  SH2ADD,SH3ADD,SH4ADD,SHL,SHRS,SHRU,SWBT,SWBF,SUB,SEXTB,
  SEXTH,TBIT,TBITF,XNOP,XOR,ZEXTB,ZEXTH,ADDCG,DIVS,RSUB,
  MVL2G,MVG2L,HALT,CUSTOM,LDL,PSYSCALL,STL,GOTOL,CALLL,SYSCALL,
  RFI,RDCTRL,WRCTRL,MLSHS,CPUID,CALLABS
} opT;

typedef enum {
  mLDSB, mLDBs, mLDUB, mLDSH, mLDUH, mLDW, mSTB, mSTBs, mSTH, mSTW
} memOpT;

typedef enum {
  S_PRED, GPR, FPR, VR, CR, IMM32, LINK, CURR_PC, PKT_PC, NXTPKT_OFFS, PLUS1SYLL, MEM, IMM9, IMM12, IMM8
} regT;


typedef struct {
  regT target;
  unsigned short reg;
  unsigned char valid;
  unsigned value;
  unsigned char cluster;
} sourceReg;

typedef struct {
  regT target;
  unsigned char chk;
  unsigned short reg;
  unsigned char valid;
  unsigned char cluster;
  unsigned res;
} destReg;

typedef struct {
  unsigned target;
  targetT addr;
  unsigned data;
  opT opcode;
  int source1, source2, source3, source4, source5, source6;
  unsigned int target1, target2, target3, target4, target5;
  targetT addr2;
  unsigned data2;

  unsigned newPC;
  unsigned char newPCValid;

  /* new packet data */
  unsigned char executed;
  /*unsigned syllable;
  unsigned PC;
  unsigned immediate;
  unsigned char immediateValid;*/
  unsigned maddr;
  unsigned char maddrValid;
  sourceReg source[7];
  destReg dest[5];
} packetT;

typedef struct {
  unsigned char is;
  unsigned char cs;
  unsigned char format;
  unsigned char opc;
  unsigned char b;

  packetT packet;
} instruction;

struct memReqT {
  unsigned *pointer;
  unsigned value;
  unsigned ctrlReg;
  memOpT memOp;
  unsigned pointerV;
  struct memReqT *next;
};

#define VTHREAD_CREATE_LOCAL 1
#define VTHREAD_CREATE_REMOTE 2
#define VTHREAD_JOIN 3

struct newThreadT {
  unsigned from;
  unsigned to;
  unsigned func;
  unsigned args;
  unsigned type;
  struct newThreadT *next;
};

typedef struct mutexT {
  unsigned data;
  unsigned status;
  struct mutexT *next;
} mutexT;

/* syscall function */
void _syscall(unsigned *, systemT *, unsigned, unsigned long long);

void sigsegv_debug(int);
void sigusr1_debug(int);
void stateDump(void);
unsigned *loadBinary(char *, unsigned);
#ifdef SHM
unsigned *loadBinaryD(char *, unsigned);
#endif
int cycle(contextT *, hyperContextT *, unsigned);
int printCounts(hyperContextT *);
unsigned checkActive(void);
instructionPacket fetchInstruction(contextT *, unsigned, contextConfig *);
unsigned checkBundle(hyperContextT *, unsigned, unsigned);
instruction instructionDecode(unsigned, unsigned, hyperContextT *, systemT *, unsigned);
packetT getOp(unsigned, unsigned, unsigned, unsigned, hyperContextT *, systemT *, unsigned);
void memRequest(systemT *, unsigned *, unsigned, unsigned, memOpT, unsigned);
int memoryDump(unsigned, unsigned, unsigned *);
void newThreadRequest(unsigned, unsigned, unsigned, unsigned, systemT *, unsigned);

int serviceThreadRequests(systemT *);

void performMemoryOp(struct memReqT *, unsigned, systemT *);
void serviceMemRequest(systemT *, unsigned, unsigned, unsigned);
void serviceMemRequestNOSTALLS(systemT *, unsigned, unsigned, unsigned);
int serviceMemRequestPERFECT(systemT *, unsigned);

void returnOpcode(opT);
void stateDumpToTerminal(instruction, hyperContextT *, unsigned long long);
void printOut(instruction, instructionPacket, hyperContextT *, unsigned long long);
int setupGalaxy(void);
int freeMem(void);

#ifdef INSIZZLEAPI
/* function to step through a single cycle (all systems, context, hypercontexts */
/*typedef int gTracePacketT;*/
int insizzleAPIClock(galaxyConfigT *, hcTracePacketT gTracePacket[][MASTERCFG_CONTEXTS_MAX][MASTERCFG_HYPERCONTEXTS_MAX]);

/* read/write registers */
int insizzleAPIWrOneSGpr(unsigned, unsigned);
int insizzleAPIRdOneSGpr(unsigned, unsigned *);

int insizzleAPIRdOneLr(unsigned *);
int insizzleAPIWrOneLr(unsigned);

int insizzleAPIRdOneBr(unsigned, unsigned *);
int insizzleAPIWrOneBr(unsigned, unsigned);

/* read/write CTRL registers */
int insizzleAPIRdCtrl(unsigned *);
int insizzleAPIWrCtrl(unsigned);

/* read/write memory */
int insizzleAPIWrOneIramLocation (unsigned, unsigned);
int insizzleAPIRdOneIramLocation (unsigned, unsigned *);

int insizzleAPIWrOneDramLocation (unsigned, unsigned);
int insizzleAPIRdOneDramLocation (unsigned, unsigned *);

/* load iram/dram */
void insizzleAPILdIRAM(char *, int);
void insizzleAPILdDRAM(char *, int);

/* setup global pointers to current CPU */
int insizzleAPISetCurrent(unsigned, unsigned, unsigned, unsigned);

/* initialise system */
int insizzleAPIStubInitVtApi(galaxyConfigT *);

/* write program counter */
int insizzleAPIWrPC(unsigned);
int insizzleAPIRdPC(unsigned *);

int insizzleAPIoutputCounts(void);

extern systemT *globalS;
extern contextT *globalC;
extern hyperContextT *globalHC;
extern clusterT *globalCL;

extern unsigned globalSid, globalCid, globalHCid, globalCLid;

void driver(void);

#endif

#endif
