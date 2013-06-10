/************************************************************************************************
 *	vtapi.h 
 * VT low-level interface API header file
 * (C) 2010 V. Chouliaras
 * Proprietary and Confidential
 * vassilios@vassilios-chouliaras.com
 ************************************************************************************************/

#ifndef VTAPI_H_
#define VTAPI_H_
/* The following modes are provided
   -DHOST=USER (User comoputer acting as a host). Can access the simulator and the LE1 hardware
   -DHOST=MB (Microblaze is the host)
   -DTARGET=STUB: The target is the simulator stub compiled for the host
   -DTARGET=SIMULATOR: The target is the simulator proper
   -DTARGET=VT: The target is the VT silicon
 */
 //#define ML605
#include "global.h"
 //#define printf xil_printf /* this is needed as printf does not work on ML605 */
   
 
#include "errno-base.h"
#ifdef SCTB
#include <systemc.h>
#include "driver.h"
#endif
//#include "driver.h"
//#include "sim_stub.h"
#define VT_MUTEXES_MAX 32



typedef enum {NOHOST=0 , 	X86=1 , 	MB=2, LEON3=3 } 		hostE;
//const char hostS[4][];
//hostS[0]="NOHOST";
typedef enum {NOTARGET=0, 	SIMSTUB=1,	INSIZZLE=2, SILICON=3, SYSTEMCTB=4, RTL=5} 	targetE;
typedef struct {
	hostE host;
	targetE target1;
	targetE target2;
	targetE target3;
	
} globalTargetT;



/*
type VT_CTRL_STATUS is (READY ,
                        RUNNING ,
                        BLOCKED_MUTEX_LOCK ,
                        TERMINATED_ASYNC_HOST ,
                        TERMINATED_ASYNC,
                        TERMINATED_SYNC
                        );
*/
typedef enum {
	DEBUG=0,
	SSTEP_READY=1,
	SSTEP_RUNNING=2,
	SSTEP_BLOCKED_MUTEX_LOCK=3,
	SSTEP_TERMINATED_SYNC=4,
	READY=5,
	RUNNING=6,
	BLOCKED_MUTEX_LOCK=7,
	TERMINATED_ASYNC_HOST=8,
	TERMINATED_ASYNC=9,
	TERMINATED_SYNC=10
} vtCtrlStateE;

#define VT_RM_DEBUG_IF			0x1			/* 	Dont modify!		*/
#define AXI 					0x0 		/* 	AXI byte swapping 	*/
#define VT_BASE					0x10b00000	/* 	Debug port address
				 								in host space 		*/
#define DEBUG 0// set to 1 to enable printing of debug messages */
#define MEMTOP 0x20000
#define STACKTOP (MEMTOP-0x1000)

/************************************************************************************************
 * Legacy DEBUG I/F
 * The legacy I/F allows for 0.5MB IRAM and 0.5MB DRAM
 ************************************************************************************************/
#define DBG_BASE 0x10b00000
#define DRAM_BASE 0x10b80000
#define IRAM_DBG_BASE 0x10000000
#define DRAM_DBG_BASE 0x20000000
#define PERIPH_DBG_BASE 0x30000000
#define SPECIAL_DBG_BASE 0x00007000
// constant S_GPR_DBG_BASE integer := 16#00000000#;
#define S_GPR_DBG_BASE 0x00000000
#define CR_DBG_BASE 0x00006000
#define PC_REG 0x0
#define DBGCTRL_REG 0x8
#define CTRL_REG 0x0
#define DBGCTRL_RUN 0x5

// Special names for architected state
#define SP 1
#define PC_REG 0x0 /* written in legacy using vtApiWrSpecial */

#define DBGCMD_LEGACY_NOP 0x0
#define DBGCMD_LEGACY_RD 0x21
#define DBGCMD_LEGACY_WR 0x22
// Hardware registers section

/************************************************************************************************
 * Debug Registers
 * These registers are stored in the debug interface
 ************************************************************************************************/

#define SYSTEM_REG								0x0			/*	System register		*/
#define CONTEXT_REG								0x1			/*	System register		*/
#define HYPERCONTEXT_REG						0x2			/*	System register		*/
#define CLUSTER_REG								0x3			/*	System register		*/
#define ADDR_SPACE_REG							0x4			/*	System register		*/
#define ADDR_REG								0x5			/*	System register		*/
#define CMD_REG									0x6			/*	System register		*/
#define STATUS_REG								0x7			/*	System register		*/
#define BUSY_REG								0x8			/*	System register		*/
#define WRDATA1_REG								0x9			/*	System register		*/
#define WRDATA2_REG								0xa			/*	System register		*/
#define RDDATA1_REG								0xb			/*	System register		*/
#define RDDATA2_REG								0xc			/*	System register		*/
#define RESPONSE_REG							0xd			/*	System register		*/
#define STACK_OFFS_REG 							0xe//: integer :=    BASE + 16#38#;
#define MEM_TOP_REG								0xf// : integer :=       BASE + 16#3c#;
#define CLUST_TEMPL_REG 						0x10// : integer := BASE + 16#40#;
#define CLUST_CASM_UNIT_REG						0x11// : integer := BASE + 16#44#;

/************************************************************************************************
 * Control Registers
 * These registers are stored in each hypercontext
 ************************************************************************************************/
  #define GALAXY_CONFIG_REG 150
  #define SYSTEM_CONFIG_REG 152
  #define PERIPH_WRAP_CONFIG_REG 149
  #define SCALARSYS_CONFIG_REG 153
  #define ISA_CONFIG_REG 154
  #define CONTEXT_CONFIG_REG 155
  #define CONTEXT_CTRL_REG  167
  #define DRAM_SHARED_CONFIG0_REG 156
  #define DRAM_SHARED_CTRL_REG  166
  #define DRAM_CLUST_TEMPL_CONFIG_REG 157
  #define IFE_SIMPLE_IRAM_PRIV_CONFIG0_REG 158
  #define HCONTEXT_CONFIG_REG 159
  #define CLUST_TEMPL_STATIC_REGFILE_CONFIG_REG 160
  #define CLUST_TEMPL_SCORE_CONFIG_REG 161
  #define  HCONTEXT_CLUST_TEMPL0_REG  162
  #define  HCONTEXT_CLUST_TEMPL1_REG 163
  #define  HCONTEXT_CLUST_TEMPL_INST0_REG 164
  #define  HCONTEXT_CLUST_TEMPL_INST1_REG  165
  #define CLUST_TEMPL_CONFIG_REG 197



// DBGCMD section
#define DBGCMD_NOP					0x00
#define DBGCMD_IRAM_READ 				0x01
#define DBGCMD_IRAM_WRITE 				0x02
#define DBGCMD_DRAM_READ 				0x03
#define DBGCMD_DRAM_WRITE				0x04
#define DBGCMD_READ_CTRL				0x05
#define DBGCMD_WRITE_CTRL				0x06
#define DBGCMD_READ_PERIPH				0x07
#define DBGCMD_WRITE_PERIPH				0x08
#define DBGCMD_READ_S_GPR				0x09
#define DBGCMD_WRITE_S_GPR				0x0a
#define DBGCMD_READ_S_PRED				0x0b
#define DBGCMD_WRITE_S_PRED				0x0c
#define DBGCMD_READ_PC					0x0d
#define DBGCMD_WRITE_PC					0x0e
#define DBGCMD_READ_LR					0x0f
#define DBGCMD_WRITE_LR					0x10
#define DBGCMD_SYSRESET					0x11
#define DBGCMD_MUTEX_INIT				0x12
#define DBGCMD_MUTEX_TRYLOCK				0x13
#define DBGCMD_MUTEX_UNLOCK				0x14
#define DBGCMD_MUTEX_DESTROY				0x15
#define DBGCMD_VTHREAD_CREATE				0x20
#define DBGCMD_VTHREAD_KILL				0x21
// DBGRSP Section
#define DBGRESP_OK 						0x0//:                   integer := 16#0#;
#define DBGRESP_INVALID_SYSTEM 			0x1//:                   integer := 16#1#;
#define DBGRESP_INVALID_CONTEXT			0x2// :                   integer := 16#2#;
#define DBGRESP_INVALID_HYPERCONTEXT	0x3// :                   integer := 16#3#;
#define DBGRESP_DBG_FSM_BUSY			0x4// :                   integer := 16#4#;
#define DBGRESP_HC_FSM_BUSY				0x5// :                   integer := 16#5#;
#define DBGRESP_INVALID_DBG_CMD			0x6// :                   integer := 16#6#;
#define DBGRESP_INVALID_REG				0x7// :                   integer := 16#7#;
#define DBGRESP_ADDR_RANGE_ERROR		0x8// :     integer := CINT8;
#define DBGRESP_UNALIGNED_ADDR_ERROR	0x9// : integer := CINT9;
#define DBGRESP_PERIPH_TIMEOUT			0xa// :       integer := CINT10;

// Maximum system parameters
#define SYSTEMS_MAX 			0x1
#define CONTEXTS_MAX			0x2 /* FIXME */
#define HYPERCONTEXTS_MAX		0x1
#define CLUSTERS_MAX			0x1
#define S_GPR_FILE_SIZE_MAX		0x40	/* Max Static GPRs */
#define S_PRED_FILE_SIZE_MAX	0x08	/* Max Static predicates */
#define MUTEXES_MAX				0x20	/* Current maximum of hardware mutex objects */

/* The following constants identify the systems parameters in the simulator */
//=============================================================================
// Major system parameters (MAX)
#define MASTERCFG_SYSTEMS_MAX 1
#define MASTERCFG_CONTEXTS_MAX 0x4
#define MASTERCFG_CLUST_TEMPL_MAX 0x1
#define MASTERCFG_HYPERCONTEXTS_MAX 0x1

#define MASTERCFG_SCALARSYS_PRESENT 0
#define MASTERCFG_PERIPH_PRESENT 0
#define MASTERCFG_IARCH IRAM_PRIVATE

#define MASTERCFG_CLUSTERS 1
#define MASTERCFG_C_ISSUE_WIDTH 4
#define MASTERCFG_CL_ISSUE_WIDTH 4
#define MASTERCFG_IALUS_PER_CLUSTER 4
#define MASTERCFG_IMULTS_PER_CLUSTER 4
#define MASTERCFG_LSU_CHANNELS_PER_CLUSTER 4
#define MASTERCFG_SCORE_PRESENT 1
#define MASTERCFG_VCORE_PRESENT 0
#define MASTERCFG_FCORE_PRESENT 0
#define MASTERCFG_CCORE_PRESENT 0

#define MASTERCFG_S_GPR_FILE_SIZE 64
#define MASTERCFG_S_FPR_FILE_SIZE 0
#define MASTERCFG_S_VR_FILE_SIZE 0
#define MASTERCFG_S_PR_FILE_SIZE 8
#define MASTERCFG_R_GPR_FILE_SIZE 0
#define MASTERCFG_R_FPR_FILE_SIZE 0
#define MASTERCFG_R_VR_FILE_SIZE 0
#define MASTERCFG_R_PR_FILE_SIZE 0
#define MASTERCFG_IRAM_SIZE 0x10000 	/* bytes 	*/
#define MASTERCFG_DRAM_SIZE 0x20000	/* bytes	*/



/*	API Error codes */
#define SUCCESS 	     0x0
#define FAILURE 	     0x1
#define INVALIDHOSTTARGET    0x2
#define INVALIDHOST	     0x4
#define INVALIDTARGET	     0x5
#define IRAMCYCLEERROR	     0x6
#define DRAMCYCLEERROR	     0x7
#define MEMDUMPALLOCATEERROR 0x8
#define CHKGALAXYCONFIGERROR 0x10 //chkGalaxyConfig function error
#define INVALIDHOSTMODE      0x11
#define EXTRACT_STATS_ERROR  0x12
/*	InSizzle Error Codes */
#define INSIZZLE_VTAPI_INIT_FAILURE 	0x100
#define INSIZZLE_EXTRACT_STATS_ERROR 	0x101

#define ERROR_VT_CANT_ACCESS_DEBUG_PORT	0x2 

#define DIAG_CTRL_SPACE			0x1											/*	Test the CTRL space of all VTs in the galaxy 		*/
#define DIAG_PERIPH_SPACE		0x0											/*	Test the PERIPH space of all VTs in the galaxy 		*/
#define DIAG_REGS				0x1											/*	Test the registers of all VTs in the galaxy 		*/

typedef enum  {
	IRAM_PRIVATE = 0,
	L1IACHE_PRIVATE_IRAM_SHARED = 1,
	L1ICACHE_PRIVATE_HOST_IRAM = 2,
	L1ICACHE_PRIVATE_L2ICACHE_HOST_IRAM = 3
	} iArchT ;
	
// system configuration structures
typedef struct {
	int s_gpr[MASTERCFG_S_GPR_FILE_SIZE];
	int s_pred[MASTERCFG_S_PR_FILE_SIZE];
} clusterConfig1T;

#define CLUSTER_CONFIG_REG_SCORE_PRESENT_SHIFT  0x00		/*	shift-by value to extract field						*/
#define CLUSTER_CONFIG_REG_SCORE_PRESENT_MASK						0x01		/*	shift-by value to extract field						*/
#define CLUSTER_CONFIG_REG_FCORE_PRESENT_SHIFT						0x01		/*	shift-by value to extract field						*/
#define CLUSTER_CONFIG_REG_FCORE_PRESENT_MASK						0x01		/*	shift-by value to extract field						*/
#define CLUSTER_CONFIG_REG_VCORE_PRESENT_SHIFT						0x02		/*	shift-by value to extract field						*/
#define CLUSTER_CONFIG_REG_VCORE_PRESENT_MASK						0x01		/*	shift-by value to extract field						*/
#define CLUSTER_CONFIG_REG_CCORE_PRESENT_SHIFT						0x03		/*	shift-by value to extract field						*/
#define CLUSTER_CONFIG_REG_CCORE_PRESENT_MASK						0x01		/*	shift-by value to extract field						*/
#define CLUSTER_CONFIG_REG_ISSUE_WIDTH_SHIFT						0x0f		/*	shift-by value to extract field						*/
#define CLUSTER_CONFIG_REG_ISSUE_WIDTH_MASK							0xff		/*	shift-by value to extract field						*/


typedef struct {
  int sCorePresent;															/*	CLUSTER_CONFIG_REG.SCORE_PRESENT					*/
  int fCorePresent;															/*	CLUSTER_CONFIG_REG.FCORE_PRESENT					*/
  int vCorePresent;															/*	CLUSTER_CONFIG_REG.VCORE_PRESENT					*/
  int cCorePresent;															/*	CLUSTER_CONFIG_REG.CCORE_PRESENT					*/
  int issueWidth;															/*	CLUSTER_CONFIG_REG.ISSUE_WIDTH					*/
  int iAlus;																	/*	CLUSTER_SCORE_CONFIG_REG.IALUS						*/
  int iMults;																	/*	CLUSTER_SCORE_CONFIG_REG.IMULTS						*/
  int lsuChannels;															/*	CLUSTER_SCORE_CONFIG_REG.LSU_CHANNELS				*/
} clusterConfigT;

#define HYPERCONTEXT_STATIC_REGFILE_CONFIG_REG_S_GPR_FILE_SIZE_SHIFT		0x00		/*	shift-by value to extract field						*/
#define HYPERCONTEXT_STATIC_REGFILE_CONFIG_REG_S_GPR_FILE_SIZE_MASK		0xff		/*	shift-by value to extract field						*/
#define HYPERCONTEXT_STATIC_REGFILE_CONFIG_REG_S_FPR_FILE_SIZE_SHIFT		0x08		/*	shift-by value to extract field						*/
#define HYPERCONTEXT_STATIC_REGFILE_CONFIG_REG_S_FPR_FILE_SIZE_MASK		0xff		/*	shift-by value to extract field						*/
#define HYPERCONTEXT_STATIC_REGFILE_CONFIG_REG_S_VR_FILE_SIZE_SHIFT		0x10		/*	shift-by value to extract field						*/
#define HYPERCONTEXT_STATIC_REGFILE_CONFIG_REG_S_VR_FILE_SIZE_MASK		0xff		/*	shift-by value to extract field						*/
#define HYPERCONTEXT_STATIC_REGFILE_CONFIG_REG_S_PR_FILE_SIZE_SHIFT		0x18		/*	shift-by value to extract field						*/
#define HYPERCONTEXT_STATIC_REGFILE_CONFIG_REG_S_PR_FILE_SIZE_MASK		0x0f		/*	shift-by value to extract field						*/

#define HYPERCONTEXT_ROTATING_REGFILE_CONFIG_REG_R_GPR_FILE_SIZE_SHIFT		0x00		/*	shift-by value to extract field						*/
#define HYPERCONTEXT_ROTATING_REGFILE_CONFIG_REG_R_GPR_FILE_SIZE_MASK		0xff		/*	shift-by value to extract field						*/
#define HYPERCONTEXT_ROTATING_REGFILE_CONFIG_REG_R_FPR_FILE_SIZE_SHIFT		0x08		/*	shift-by value to extract field						*/
#define HYPERCONTEXT_ROTATING_REGFILE_CONFIG_REG_R_FPR_FILE_SIZE_MASK		0xff		/*	shift-by value to extract field						*/
#define HYPERCONTEXT_ROTATING_REGFILE_CONFIG_REG_R_VR_FILE_SIZE_SHIFT		0x10		/*	shift-by value to extract field						*/
#define HYPERCONTEXT_ROTATING_REGFILE_CONFIG_REG_R_VR_FILE_SIZE_MASK		0xff		/*	shift-by value to extract field						*/
#define HYPERCONTEXT_ROTATING_REGFILE_CONFIG_REG_R_PR_FILE_SIZE_SHIFT		0x18		/*	shift-by value to extract field						*/
#define HYPERCONTEXT_ROTATING_REGFILE_CONFIG_REG_R_PR_FILE_SIZE_MASK		0x0f		/*	shift-by value to extract field						*/
//=============================================================================
// GALAXY_CONFIG_REG[1]
#define GALAXY_CONFIG_REG_SYSTEMS_SHIFT	0x0							/*	shift-by value to extract field						*/
#define GALAXY_CONFIG_REG_SYSTEMS_MASK	0xff						/*	bit-mask for given field							*/
#define GALAXY_CONFIG_REG_ICARCH_SHIFT	0x8							/*	shift-by value to extract field						*/
#define GALAXY_CONFIG_REG_ICARCH_MASK	0xf						/*	bit-mask for given field							*/
#define GALAXY_CONFIG_REG_VT_RM_DEBUG_IF_SHIFT	0x1f							/*	shift-by value to extract field						*/
#define GALAXY_CONFIG_REG_VT_RM_DEBUG_IF_MASK	0x1						/*	bit-mask for given field							*/
//=============================================================================
// SYSTEM_CONFIG_REG[SYSTEMS_MAX]
#define SYSTEM_CONFIG_REG_CONTEXTS_SHIFT			0x00			/*	shift-by value to extract field						*/
#define SYSTEM_CONFIG_REG_CONTEXTS_MASK		      	0xff			/*	bit-mask for given field							*/
#define SYSTEM_CONFIG_REG_SCALARSYS_PRESENT_SHIFT	0x08			/*	shift-by value to extract field						*/
#define SYSTEM_CONFIG_REG_SCALARSYS_PRESENT_MASK	0x01			/*	bit-mask for given field							*/
#define SYSTEM_CONFIG_REG_PERIPH_PRESENT_SHIFT		0x0a			/*	shift-by value to extract field						*/
#define SYSTEM_CONFIG_REG_PERIPH_PRESENT_MASK		0x01			/*	bit-mask for given field							*/
#define SYSTEM_CONFIG_REG_DARCH_SHIFT				0x0b			/*	shift-by value to extract field						*/
#define SYSTEM_CONFIG_REG_DARCH_MASK				0x0f			/*	bit-mask for given field							*/
//=============================================================================
// PERIPH_WRAP_CONFIG_REG
#define PERIPH_WRAP_CONFIG_REG_PERIPH_SHIFT 		0X0
#define PERIPH_WRAP_CONFIG_REG_PERIPH_MASK			0XFFFF
//=============================================================================
// DRAM_SHARED_CONFIG0_REG
#define DRAM_SHARED_CONFIG0_REG_DRAM_BLK_SIZE_SHIFT 0x0
#define DRAM_SHARED_CONFIG0_REG_DRAM_BLK_SIZE_MASK 0x8
#define DRAM_SHARED_CONFIG0_REG_DRAM_SIZE_SHIFT 0x8
#define DRAM_SHARED_CONFIG0_REG_DRAM_SIZE_MASK 0xffff
#define DRAM_SHARED_CONFIG0_REG_DRAM_BANKS_SHIFT 0x18
#define DRAM_SHARED_CONFIG0_REG_DRAM_BANKS_MASK 0x8
//=============================================================================
// CONTEXT_CONFIG_REG
#define CONTEXT_CONFIG_REG_ISA_PRSPCTV_SHIFT			0x00						/*	shift-by value to extract field						*/
#define CONTEXT_CONFIG_REG_ISA_PRSPCTV_MASK			0x0F						/*	bit-mask for given field							*/
#define CONTEXT_CONFIG_REG_HCONTEXTS_SHIFT		0x04						/*	shift-by value to extract field						*/
#define CONTEXT_CONFIG_REG_HCONTEXTS_MASK		0x0f						/*	bit-mask for given field							*/
#define CONTEXT_CONFIG_REG_CLUST_TEMPL_SHIFT			0x08						/*	shift-by value to extract field						*/
#define CONTEXT_CONFIG_REG_CLUST_TEMPL_MASK			0x0f						/*	bit-mask for given field							*/
#define CONTEXT_CONFIG_REG_C_ISSUE_WIDTH_MAX_SHIFT			0x0c						/*	shift-by value to extract field						*/
#define CONTEXT_CONFIG_REG_C_ISSUE_WIDTH_MAX_MASK			0xff						/*	bit-mask for given field							*/
#define CONTEXT_CONFIG_REG_IARCH_SHIFT			0x14						/*	shift-by value to extract field						*/
#define CONTEXT_CONFIG_REG_IARCH_MASK			0x0f						/*	bit-mask for given field							*/
//=============================================================================
// IFE_SIMPLE_IRAM_PRIV_CONFIG0_REG
#define IFE_SIMPLE_IRAM_PRIV_CONFIG0_REG_IFETCH_WIDTH_SHIFT 0x0
#define IFE_SIMPLE_IRAM_PRIV_CONFIG0_REG_IFETCH_WIDTH_MASK 0xff
#define IFE_SIMPLE_IRAM_PRIV_CONFIG0_REG_IRAM_SIZE_SHIFT 0x8
#define IFE_SIMPLE_IRAM_PRIV_CONFIG0_REG_IRAM_SIZE_MASK 0xffff
//=============================================================================
// CLUST_TEMPL_CONFIG_REG
#define CLUST_TEMPL_CONFIG_REG_SCORE_PRESENT_SHIFT 0x0
#define CLUST_TEMPL_CONFIG_REG_SCORE_PRESENT_MASK 0x1
#define CLUST_TEMPL_CONFIG_REG_VCORE_PRESENT_SHIFT 0x1
#define CLUST_TEMPL_CONFIG_REG_VCORE_PRESENT_MASK 0x1
#define CLUST_TEMPL_CONFIG_REG_FPCORE_PRESENT_SHIFT 0x2
#define CLUST_TEMPL_CONFIG_REG_FPCORE_PRESENT_MASK 0x1
#define CLUST_TEMPL_CONFIG_REG_CCORE_PRESENT_SHIFT 0x3
#define CLUST_TEMPL_CONFIG_REG_CCORE_PRESENT_MASK 0x1
#define CLUST_TEMPL_CONFIG_REG_CL_ISSUE_WIDTH_SHIFT 0x8
#define CLUST_TEMPL_CONFIG_REG_CL_ISSUE_WIDTH_MASK 0xff
#define CLUST_TEMPL_CONFIG_REG_INSTANTIATE_SHIFT 0x10
#define CLUST_TEMPL_CONFIG_REG_INSTANTIATE_MASK 0x1
#define CLUST_TEMPL_CONFIG_REG_INSTANCES_SHIFT 0x11
#define CLUST_TEMPL_CONFIG_REG_INSTANCES_MASK 0xf
//=============================================================================
// CLUST_TEMPL_STATIC_REGFILE_CONFIG_REG
#define CLUST_TEMPL_STATIC_REGFILE_CONFIG_REG_S_GPR_FILE_SIZE_SHIFT		0x00		/*	shift-by value to extract field						*/
#define CLUST_TEMPL_STATIC_REGFILE_CONFIG_REG_S_GPR_FILE_SIZE_MASK		0xff		/*	shift-by value to extract field						*/
#define CLUST_TEMPL_STATIC_REGFILE_CONFIG_REG_S_FPR_FILE_SIZE_SHIFT		0x08		/*	shift-by value to extract field						*/
#define CLUST_TEMPL_STATIC_REGFILE_CONFIG_REG_S_FPR_FILE_SIZE_MASK		0xff		/*	shift-by value to extract field						*/
#define CLUST_TEMPL_STATIC_REGFILE_CONFIG_REG_S_VR_FILE_SIZE_SHIFT		0x10		/*	shift-by value to extract field						*/
#define CLUST_TEMPL_STATIC_REGFILE_CONFIG_REG_S_VR_FILE_SIZE_MASK		0xff		/*	shift-by value to extract field						*/
#define CLUST_TEMPL_STATIC_REGFILE_CONFIG_REG_S_PR_FILE_SIZE_SHIFT		0x18		/*	shift-by value to extract field						*/
#define CLUST_TEMPL_STATIC_REGFILE_CONFIG_REG_S_PR_FILE_SIZE_MASK		0x0f		/*	shift-by value to extract field						*/
//=============================================================================
// CLUST_TEMPL_SCORE_CONFIG_REG
#define CLUST_TEMPL_SCORE_CONFIG_REG_IALUS_SHIFT  0x0
#define CLUST_TEMPL_SCORE_CONFIG_REG_IALUS_MASK  0xff
#define CLUST_TEMPL_SCORE_CONFIG_REG_IMULTS_SHIFT  0x8
#define CLUST_TEMPL_SCORE_CONFIG_REG_IMULTS_MASK  0xff
#define CLUST_TEMPL_SCORE_CONFIG_REG_LSU_CHANNELS_SHIFT  0x10
#define CLUST_TEMPL_SCORE_CONFIG_REG_LSU_CHANNELS_MASK  0xff
#define CLUST_TEMPL_SCORE_CONFIG_REG_BRUS_SHIFT  0x18
#define CLUST_TEMPL_SCORE_CONFIG_REG_BRUS_MASK  0xff
//=============================================================================
// HCONTEXT_CLUST_TEMPL0_REG
#define HCONTEXT_CLUST_TEMPL0_REG_CL0_TEMPL_SHIFT 0x0
#define HCONTEXT_CLUST_TEMPL0_REG_CL0_TEMPL_MASK 0xf
#define HCONTEXT_CLUST_TEMPL0_REG_CL1_TEMPL_SHIFT 0x4
#define HCONTEXT_CLUST_TEMPL0_REG_CL1_TEMPL_MASK 0xf
#define HCONTEXT_CLUST_TEMPL0_REG_CL2_TEMPL_SHIFT 0x8
#define HCONTEXT_CLUST_TEMPL0_REG_CL2_TEMPL_MASK 0xf
#define HCONTEXT_CLUST_TEMPL0_REG_CL3_TEMPL_SHIFT 0xc
#define HCONTEXT_CLUST_TEMPL0_REG_CL3_TEMPL_MASK 0xf
#define HCONTEXT_CLUST_TEMPL0_REG_CL4_TEMPL_SHIFT 0x10
#define HCONTEXT_CLUST_TEMPL0_REG_CL4_TEMPL_MASK 0xf
#define HCONTEXT_CLUST_TEMPL0_REG_CL5_TEMPL_SHIFT 0x14
#define HCONTEXT_CLUST_TEMPL0_REG_CL5_TEMPL_MASK 0xf
#define HCONTEXT_CLUST_TEMPL0_REG_CL6_TEMPL_SHIFT 0x18
#define HCONTEXT_CLUST_TEMPL0_REG_CL6_TEMPL_MASK 0xf
#define HCONTEXT_CLUST_TEMPL0_REG_CL7_TEMPL_SHIFT 0x1c
#define HCONTEXT_CLUST_TEMPL0_REG_CL7_TEMPL_MASK 0xf
//=============================================================================
// HCONTEXT_CLUST_TEMPL1_REG
#define HCONTEXT_CLUST_TEMPL1_REG_CL8_TEMPL_SHIFT 0x0
#define HCONTEXT_CLUST_TEMPL1_REG_CL8_TEMPL_MASK 0xf
#define HCONTEXT_CLUST_TEMPL1_REG_CL9_TEMPL_SHIFT 0x4
#define HCONTEXT_CLUST_TEMPL1_REG_CL9_TEMPL_MASK 0xf
#define HCONTEXT_CLUST_TEMPL1_REG_CL10_TEMPL_SHIFT 0x8
#define HCONTEXT_CLUST_TEMPL1_REG_CL10_TEMPL_MASK 0xf
#define HCONTEXT_CLUST_TEMPL1_REG_CL11_TEMPL_SHIFT 0xc
#define HCONTEXT_CLUST_TEMPL1_REG_CL11_TEMPL_MASK 0xf
#define HCONTEXT_CLUST_TEMPL1_REG_CL12_TEMPL_SHIFT 0x10
#define HCONTEXT_CLUST_TEMPL1_REG_CL12_TEMPL_MASK 0xf
#define HCONTEXT_CLUST_TEMPL1_REG_CL13_TEMPL_SHIFT 0x14
#define HCONTEXT_CLUST_TEMPL1_REG_CL13_TEMPL_MASK 0xf
#define HCONTEXT_CLUST_TEMPL1_REG_CL14_TEMPL_SHIFT 0x18
#define HCONTEXT_CLUST_TEMPL1_REG_CL14_TEMPL_MASK 0xf
#define HCONTEXT_CLUST_TEMPL1_REG_CL15_TEMPL_SHIFT 0x1c
#define HCONTEXT_CLUST_TEMPL1_REG_CL15_TEMPL_MASK 0xf
//=============================================================================
// HCONTEXT_CLUST_TEMPL_INST0_REG
#define HCONTEXT_CLUST_TEMPL_INST0_REG_CL0_TEMPL_INST_SHIFT 0x0
#define HCONTEXT_CLUST_TEMPL_INST0_REG_CL0_TEMPL_INST_MASK 0xf
#define HCONTEXT_CLUST_TEMPL_INST0_REG_CL1_TEMPL_INST_SHIFT 0x0
#define HCONTEXT_CLUST_TEMPL_INST0_REG_CL1_TEMPL_INST_MASK 0xf
#define HCONTEXT_CLUST_TEMPL_INST0_REG_CL2_TEMPL_INST_SHIFT 0x0
#define HCONTEXT_CLUST_TEMPL_INST0_REG_CL2_TEMPL_INST_MASK 0xf
#define HCONTEXT_CLUST_TEMPL_INST0_REG_CL3_TEMPL_INST_SHIFT 0x0
#define HCONTEXT_CLUST_TEMPL_INST0_REG_CL3_TEMPL_INST_MASK 0xf
#define HCONTEXT_CLUST_TEMPL_INST0_REG_CL4_TEMPL_INST_SHIFT 0x0
#define HCONTEXT_CLUST_TEMPL_INST0_REG_CL4_TEMPL_INST_MASK 0xf
#define HCONTEXT_CLUST_TEMPL_INST0_REG_CL5_TEMPL_INST_SHIFT 0x0
#define HCONTEXT_CLUST_TEMPL_INST0_REG_CL5_TEMPL_INST_MASK 0xf
#define HCONTEXT_CLUST_TEMPL_INST0_REG_CL6_TEMPL_INST_SHIFT 0x0
#define HCONTEXT_CLUST_TEMPL_INST0_REG_CL6_TEMPL_INST_MASK 0xf
#define HCONTEXT_CLUST_TEMPL_INST0_REG_CL7_TEMPL_INST_SHIFT 0x0
#define HCONTEXT_CLUST_TEMPL_INST0_REG_CL7_TEMPL_INST_MASK 0xf
//=============================================================================
// HCONTEXT_CLUST_TEMPL_INST1_REG
#define HCONTEXT_CLUST_TEMPL_INST1_REG_CL8_TEMPL_INST_SHIFT 0x0
#define HCONTEXT_CLUST_TEMPL_INST1_REG_CL8_TEMPL_INST_MASK 0xf
#define HCONTEXT_CLUST_TEMPL_INST1_REG_CL9_TEMPL_INST_SHIFT 0x0
#define HCONTEXT_CLUST_TEMPL_INST1_REG_CL9_TEMPL_INST_MASK 0xf
#define HCONTEXT_CLUST_TEMPL_INST1_REG_CL10_TEMPL_INST_SHIFT 0x0
#define HCONTEXT_CLUST_TEMPL_INST1_REG_CL10_TEMPL_INST_MASK 0xf
#define HCONTEXT_CLUST_TEMPL_INST1_REG_CL11_TEMPL_INST_SHIFT 0x0
#define HCONTEXT_CLUST_TEMPL_INST1_REG_CL11_TEMPL_INST_MASK 0xf
#define HCONTEXT_CLUST_TEMPL_INST1_REG_CL12_TEMPL_INST_SHIFT 0x0
#define HCONTEXT_CLUST_TEMPL_INST1_REG_CL12_TEMPL_INST_MASK 0xf
#define HCONTEXT_CLUST_TEMPL_INST1_REG_CL13_TEMPL_INST_SHIFT 0x0
#define HCONTEXT_CLUST_TEMPL_INST1_REG_CL13_TEMPL_INST_MASK 0xf
#define HCONTEXT_CLUST_TEMPL_INST1_REG_CL14_TEMPL_INST_SHIFT 0x0
#define HCONTEXT_CLUST_TEMPL_INST1_REG_CL14_TEMPL_INST_MASK 0xf
#define HCONTEXT_CLUST_TEMPL_INST1_REG_CL15_TEMPL_INST_SHIFT 0x0
#define HCONTEXT_CLUST_TEMPL_INST1_REG_CL15_TEMPL_INST_MASK 0xf


typedef struct {
	  vtCtrlStateE VT_CTRL[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_HYPERCONTEXTS_MAX]; /* VT_CTRL_REG */
	  int GALAXY_CONFIG;
	  int SYSTEM_CONFIG[MASTERCFG_SYSTEMS_MAX];
	  int PERIPH_WRAP_CONFIG[MASTERCFG_SYSTEMS_MAX];
	  int DRAM_SHARED_CONFIG0[MASTERCFG_SYSTEMS_MAX];
	  int DRAM_SHARED_CTRL[MASTERCFG_SYSTEMS_MAX];
	  int VTHREADS_CONFIG[MASTERCFG_SYSTEMS_MAX];
	  int CONTEXT_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX];
	  int CONTEXT_CTRL[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX];	  
	  int IFE_SIMPLE_IRAM_PRIV_CONFIG0[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX];
	  int CLUST_TEMPL_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int HCONTEXT_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_HYPERCONTEXTS_MAX];
	  int CLUST_TEMPL_STATIC_REGFILE_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_SCORE_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_CCORE7_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_CCORE6_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_CCORE5_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_CCORE4_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_CCORE3_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_CCORE2_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];

	  int CLUST_TEMPL_CASM6X1_XISA0_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_CASM6X1_XISA1_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_CASM5X2_XISA0_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_CASM5X2_XISA1_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_CASM4X3_XISA0_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_CASM4X3_XISA1_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_CASM3X4_XISA0_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_CASM3X4_XISA1_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];

	  int CLUST_TEMPL_CASM5X1_XISA0_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_CASM5X1_XISA1_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_CASM4X2_XISA0_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_CASM4X2_XISA1_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_CASM3X3_XISA0_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_CASM3X3_XISA1_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_CASM2X4_XISA0_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_CASM2X4_XISA1_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  
	  int CLUST_TEMPL_CASM4X1_XISA0_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_CASM4X1_XISA1_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_CASM3X2_XISA0_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_CASM3X2_XISA1_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_CASM2X3_XISA0_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_CASM2X3_XISA1_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];

	  int CLUST_TEMPL_CASM3X1_XISA0_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_CASM3X1_XISA1_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_CASM2X2_XISA0_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_CASM2X2_XISA1_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_CASM1X3_XISA0_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_CASM1X3_XISA1_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];

	  int CLUST_TEMPL_CASM2X1_XISA0_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_CASM2X1_XISA1_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_CASM1X2_XISA0_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_CASM1X2_XISA1_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];

	  int CLUST_TEMPL_CASM1X1_XISA0_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  int CLUST_TEMPL_CASM1X1_XISA1_CONFIG[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_CLUST_TEMPL_MAX];
	  	  
	  int HCONTEXT_CLUST_TEMPL0[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_HYPERCONTEXTS_MAX];
	  int HCONTEXT_CLUST_TEMPL1[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_HYPERCONTEXTS_MAX];

	  int HCONTEXT_CLUST_TEMPL_INST0[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_HYPERCONTEXTS_MAX];
	  int HCONTEXT_CLUST_TEMPL_INST1[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_HYPERCONTEXTS_MAX];

} ctrlStateT;

typedef struct {
	int sGpr[MASTERCFG_CLUST_TEMPL_MAX][S_GPR_FILE_SIZE_MAX];
	int sPr [MASTERCFG_CLUST_TEMPL_MAX][S_PRED_FILE_SIZE_MAX];
	int lr;
	int pc;
} oneArchStateT;


typedef struct {
	oneArchStateT oneArchState[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_HYPERCONTEXTS_MAX];
	int * iRam[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX];
	int * dRam[MASTERCFG_SYSTEMS_MAX];
} fullArchStateT;


typedef struct {
  int system_reg;
  int context_reg;
  int hypercontext_reg;
  int cluster_reg;
  int addr_reg;
  int busy_reg;
  int wrdata1_reg;
  int wrdata2_reg;
  int rddata1_reg;
  int rddata2_reg;
  int response_reg;
  int streaming_ctrl_reg;
} debugStateT;

// These are the ISA perspectives
#define VT32PP 0x0
#define VT64PP 0x1
#define VT32EPIC 0x2
#define VT64EPIC 0x3



  typedef struct {
    ctrlStateT 	ctrlState;
    debugStateT 	debugState;
    fullArchStateT galaxyConfigT;
    } galaxyConfigT;


typedef struct {
  int sys;
  int c;
  int hc;
  int cl;
} currentT;



/* Destroy a mutex.  */
//extern int pthread_mutex_destroy (pthread_mutex_t *__mutex);


/* Try locking a mutex.  */
//extern int pthread_mutex_trylock (pthread_mutex_t *__mutex);
  

/* Lock a mutex.  */ 
//extern int pthread_mutex_lock (pthread_mutex_t *__mutex);



/* Wait until lock becomes available, or specified time passes. */
//extern int pthread_mutex_timedlock (pthread_mutex_t *__restrict __mutex,  __const struct timespec *__restrict__abstime) __THROW __nonnull ((1, 2));#endif

/* Unlock a mutex.  */
//extern int pthread_mutex_unlock (pthread_mutex_t *__mutex);

 


/* Host-side functions * 
 */

/* Initialize a mutex.  */
//extern int host_vthread_mutex_init (pthread_mutex_t *__mutex,    __const pthread_mutexattr_t *__mutexattr);

/* User exports */
int vtApiLoadIram(galaxyConfigT *galaxyConfig , unsigned int system , unsigned int context , char * iramBin , unsigned int iramLen, hostE host , targetE target);

int vtApiLoadDram(galaxyConfigT *galaxyConfig , unsigned int system , char * dramBin, unsigned int dramLen , hostE host, targetE target);

int vtApiDumpDram(galaxyConfigT *galaxyConfig , unsigned int system , unsigned int * dump, hostE host, targetE target);
int vtApiDma  (int * buf1 , int *  buf2  , unsigned int bsize ,globalTargetT globalTarget );


int vtApiDumpArchState(galaxyConfigT *galaxyConfig , unsigned int system , unsigned int context ,unsigned int hypercontext, fullArchStateT * fullArchStateDump , hostE host , targetE target );

int vtApiDumpOneArchState(galaxyConfigT *galaxyConfig , unsigned int system , unsigned int context ,unsigned int hypercontext, oneArchStateT * oneArchStateDump , hostE host , targetE target);

int vtDbgsetCurrent(unsigned int system , unsigned int context , unsigned int hypercontext,  unsigned int cluster , hostE host , targetE target);
// Write one IRAM location from addressed S.C
int wrOneIramLocation (galaxyConfigT *galaxyConfig, unsigned int system , unsigned int context , unsigned int iaddr , unsigned int data, targetE target);
// Read one IRAM location from addressed S.C. Data returned in int * data
int rdOneIramLocation (galaxyConfigT *galaxyConfig, unsigned int system , unsigned int context , unsigned int iaddr , unsigned int * data, targetE target);
int vtApiStartOneHc(galaxyConfigT *galaxyConfig, unsigned int system , unsigned int context , unsigned int hc , unsigned int pc , unsigned int sp, hostE host , targetE target);
int vtApiWaitOneHc(galaxyConfigT *galaxyConfig, unsigned int system , unsigned int context , unsigned int hc ,  hostE host , targetE target);
int vtApiForkMultiCHC0(galaxyConfigT *galaxyConfig, unsigned int system , unsigned int contexts ,  unsigned int contextOffset, unsigned int pc , unsigned int spOffset,  char * iramBin , unsigned int iramLen, char * dramBin, unsigned int dramLen , hostE host , targetE target);
int vtApiJoinMultiCHC0(galaxyConfigT *galaxyConfig, unsigned int system , unsigned int contexts ,unsigned int contextOffset, hostE host , targetE target);

int vtDbgRd (unsigned int addr , unsigned int * rdata, targetE target);
int vtDbgWr (unsigned int addr , unsigned  int wrdata , targetE target);

int setCtrlStatus (galaxyConfigT *galaxyConfig , int system , int context , int hc , vtCtrlStateE vtCtrlState , hostE host , targetE target);

int vtApiWaitOneHc(galaxyConfigT *galaxyConfig,unsigned int system , unsigned int context , unsigned int hc,  hostE host , targetE target);





//#ifdef SCTB


  typedef enum 
    {
      NO_TRIGGER,                               // No trigger
      C_CLOCKS,                                 // Single clock
      HC_COMMITED_INSTRUCTIONS,                 // Instructions commited. IPC=
                                                // HC_COMMITED_INSTRUCTIONS/C_CLOCKS
      HC_STATE_DEBUG,                           // hc in debug state (prm1.1)
      HC_STATE_DEBUG_CLOCKS,                    // No of clocks in this state
      HC_STATE_SSTEP_READY,                     // hc in SSTEP_READY state
      HC_STATE_SSTEP_READY_CLOCKS,              // No of clocks in this state
      HC_STATE_SSTEP_RUNNING,                   // hc in SSTEP_RUNNING state
      HC_STATE_SSTEP_RUNNING_CLOCKS,            // No of clocks in this state
      HC_STATE_SSTEP_BLOCKED_MUTEX_LOCK,        //
      HC_STATE_SSTEP_BLOCKED_MUTEX_LOCK_CLOCKS, //
      HC_STATE_SSTEP_TERMINATED_SYNC,           //
      HC_STATE_SSTEP_TERMINATED_SYNC_CLOCKS,    //
      HC_STATE_READY,                           //
      HC_STATE_READY_CLOCKS,                    //
      HC_STATE_RUNNING,                         //
      HC_STATE_RUNNING_CLOCKS,                  //
      HC_STATE_BLOCKED_MUTEX_LOCK,              //
      HC_STATE_BLOCKED_MUTEX_LOCK_CLOCKS,       //
      HC_STATE_TERMINATED_ASYNC_HOST,           //
      HC_STATE_TERMINATED_ASYNC_HOST_CLOCKS,    //
      HC_STATE_TERMINATED_SYNC,                 //
      HC_STATE_TERMINATED_SYNC_CLOCKS,          //
      HC_PIPE_RESTART_BRANCH,                   // Pipeline restart due to branch
      HC_PIPE_RESTART_CALL,                     // Pipeline restart due to call
      HC_PIPE_RESTART_RET,                      // Pipeline restart dur to RET
      HC_IFEI_STALL,                             // Core-initiated STALL to the
                                                // IFE
      HC_IFE_NO_FETCH,                          // IFE cant deliver instruction
                                                // packets (ifeo.ivalid=0).
                                                // Irrespective of restart3_i
                                                // and ifei.stall. HC must be
                                                // running (HC_STATE=HC_STATE_RUNNING)
      HC_LSU_HOLD,                              // LSU-initiated stall
      HC_SCRBD_S_GPR_STALL,                     // S_GPR dependency stall event
      HC_SCRBD_S_BR_STALL,                      // S_BR dependency stall event
      HC_SCRBD_S_LR_STALL,                      // Link Register dependency
                                                // stall event
      HC_SCRBD_STALL ,                          // Scoreboard stall event
	SYSTEM_STATE_RUNNING_CLOCKS,
	HC_SYSTEM_STATE_RUNNING_OFFSET
      
    } instrTriggerT;
    #define PERIPH_USER0_REG 28 /* THIS IS AN IMPORTANT OFFSET!!!! */
    #define INSTR_COUNTERS_MAX 32
    #define INSTR_CTRL_REG_BASE  (0x0 + PERIPH_USER0_REG)
    #define INSTR_COUNTER_REG_BASE (INSTR_CTRL_REG_BASE + INSTR_COUNTERS_MAX)
    #define INSTR_CONFIG_REG  (INSTR_COUNTER_REG_BASE + INSTR_COUNTERS_MAX)
  	#define INSTR_CTRL_REG_CONTEXT_SHIFT 24
  	#define INSTR_CTRL_REG_CONTEXT_MASK 0xff
	#define INSTR_CTRL_REG_HCONTEXT_SHIFT 20
	#define INSTR_CTRL_REG_HCONTEXT_MASK 0xf
	#define INSTR_CTRL_REG_TRIGGER_SHIFT 10
	#define INSTR_CTRL_REG_TRIGGER_MASK 1023
	#define INSTR_CTRL_REG_OVF_SHIFT 7
	#define INSTR_CTRL_REG_OVF_MASK 1
	#define INSTR_CTRL_REG_INTERNAL_SHIFT 6
	#define INSTR_CTRL_REG_INTERNAL_MASK 1
	#define INSTR_CTRL_REG_ENABLED_SHIFT 5
	#define INSTR_CTRL_REG_ENABLED_MASK 1
	#define INSTR_CTRL_REG_LINKED_SHIFT 0
	#define INSTR_CTRL_REG_LINKED_MASK 0x1f
    
// These structures are populated by Inssizle
typedef struct {
  long HC_COMMITED_INSTRUCTIONS; 	/* no of individual riscops executed */
  long HC_STATE_DEBUG;                           // hc in debug state (prm1.1)
  long HC_STATE_DEBUG_CLOCKS;                    // No of clocks in this state	
  long HC_STATE_SSTEP_READY;                     // hc in SSTEP_READY state
  long HC_STATE_SSTEP_READY_CLOCKS;              // No of clocks in this state
  long HC_STATE_SSTEP_RUNNING;                   // hc in SSTEP_RUNNING state
  long HC_STATE_SSTEP_RUNNING_CLOCKS;            // No of clocks in this state
  long HC_STATE_SSTEP_BLOCKED_MUTEX_LOCK;        //
  long HC_STATE_SSTEP_BLOCKED_MUTEX_LOCK_CLOCKS; //
  long HC_STATE_SSTEP_TERMINATED_SYNC;           //
  long HC_STATE_SSTEP_TERMINATED_SYNC_CLOCKS;    //
  long HC_STATE_READY;                           //
  long HC_STATE_READY_CLOCKS;                    //
  long HC_STATE_RUNNING;                         //
  long HC_STATE_RUNNING_CLOCKS;                  //
  long HC_STATE_BLOCKED_MUTEX_LOCK;              //
  long HC_STATE_BLOCKED_MUTEX_LOCK_CLOCKS;       //
  long HC_STATE_TERMINATED_ASYNC_HOST;           //
  long HC_STATE_TERMINATED_ASYNC_HOST_CLOCKS;    //
  long HC_STATE_TERMINATED_SYNC;                 //
  long HC_STATE_TERMINATED_SYNC_CLOCKS;          //
  long HC_PIPE_RESTART_BRANCH;                   // Pipeline restart due to branch
  long HC_PIPE_RESTART_CALL;                     // Pipeline restart due to call
  long HC_PIPE_RESTART_RET;                      // Pipeline restart dur to RET
  long HC_IFEI_STALL;                             // Core-initiated STALL to the
                                                // IFE
  long HC_IFE_NO_FETCH;                          // IFE cant deliver instruction
                                                // packets (ifeo.ivalid=0).
                                                // Irrespective of restart3_i
                                                // and ifei.stall. HC must be
                                                // running (HC_STATE=HC_STATE_RUNNING)
  long HC_LSU_HOLD;                              // LSU-initiated stall
  long HC_SCRBD_S_GPR_STALL;                     // S_GPR dependency stall event
  long HC_SCRBD_S_BR_STALL;                      // S_BR dependency stall event
  long HC_SCRBD_S_LR_STALL;                      // Link Register dependency
                                                // stall event
  long HC_SCRBD_STALL ;                          // Scoreboard stall event
      
  long HC_SYSTEM_STATE_RUNNING_OFFSET;
  long 	unsigned int SYSTEM_STATE_RUNNING_CLOCKS;

} hcStatsT;
typedef struct {
  long ifeMissedIssue;	/* No of times the LIW extended across two words */	
  long unsigned int C_CLOCKS;
} cStatsT;

typedef struct {
	long SYSTEM_STATE_RUNNING_CLOCKS;
} sStatsT;

typedef struct {
	hcStatsT hcStats[SYSTEMS_MAX][CONTEXTS_MAX][HYPERCONTEXTS_MAX];
	cStatsT cStats[SYSTEMS_MAX][CONTEXTS_MAX];
	sStatsT sStats[SYSTEMS_MAX];
} statsT;



int vtApiInit (galaxyConfigT * galaxyConfig, hostE host,targetE target);
int vtApiWrPeriph(galaxyConfigT *galaxyConfig , unsigned int system ,unsigned int periphId ,unsigned int reg,unsigned int wdata,  targetE target);
int vtApiRdPeriph(galaxyConfigT *galaxyConfig , unsigned int system , unsigned int periphId ,unsigned int reg, unsigned int *rdata, targetE target);
int vtApiSetUpInstrumentation(	galaxyConfigT *galaxyConfig , unsigned int instrPeriphId , unsigned int system , unsigned int context , unsigned int hContext , unsigned int counter , instrTriggerT trigger ,  hostE host, targetE target);
int vtApiRdInstrumentation(	galaxyConfigT * , unsigned int  , unsigned int  , unsigned int  , unsigned int  , unsigned int  , unsigned long  *  ,  instrTriggerT , hostE , targetE );

//int vtApiRdInstrumentation(	galaxyConfigT *galaxyConfig , unsigned int instrPeriphId , unsigned int system , unsigned int context , unsigned int hContext , unsigned int counter , unsigned long  * rdata ,  hostE host, targetE target);
int chkGalaxyConfig(globalTargetT globalTarget, galaxyConfigT galaxyConfig1,  galaxyConfigT  galaxyConfig2);
extern int insizzleStubInitVtApi (galaxyConfigT * galaxyConfig );
/* debug read */
int dbgRd(int);
/* get galaxy configuration */
int getGalaxyConfig (hostE host, targetE target , galaxyConfigT * galaxyConfig);

/* This is the trace packet format from the RTL

  type oneSrcRegT is record
    reg : STD_LOGIC_VECTOR(5 downto 0);
    valid : std_logic;
    val : STD_LOGIC_VECTOR(31 downto 0);
    cluster : integer;
  end record;
*/
typedef struct {
  unsigned short reg;
  unsigned char valid;
  unsigned int val;
  unsigned char cluster;
} oneSrcRegT;
/*
  type oneDstRegT is record
    chk : std_logic;                    -- This enables checking of destinations
    reg : std_logic_vector(5 downto 0);
    valid : std_logic;
    cluster : std_logic_vector(3 downto 0);
    res : std_logic_vector(31 downto 0);    
  end record;
*/
typedef struct {
  unsigned char chk;
  unsigned short reg;
  unsigned char valid;
  unsigned char cluster;
  unsigned int res;
} oneDstRegT;
/*
  type hcTracePacketT is record
    vt_ctrl : std_logic_vector(31 downto 0);  -- every HC pipes its vt_ctrl reg
    pc : STD_LOGIC_VECTOR(31 downto 0);
    imm : STD_LOGIC_VECTOR(31 downto 0);
    maddr : STD_LOGIC_VECTOR(31 downto 0);
    maddrValid : std_logic;
    rs1 : oneSrcRegT;
    rs2 : oneSrcRegT;
    rs3 : oneSrcRegT;
    rs4 : oneSrcRegT;
    rs5 : oneSrcRegT;
    rs6 : oneSrcRegT;
    rs7 : oneSrcRegT;
    rd1 : oneDstRegT;
    rd2 : oneDstRegT;
  end record;
*/
typedef struct {
  //int bundle[MASTERCFG_CL_ISSUE_WIDTH];
  unsigned char executed;
  int syll;
  int vt_ctrl;
  int pc;
  int imm;
  int maddr;
  int maddrValid;
  oneSrcRegT rs1;
  oneSrcRegT rs2;
  oneSrcRegT rs3;
  oneSrcRegT rs4;
  oneSrcRegT rs5;
  oneSrcRegT rs6;
  oneSrcRegT rs7;
  oneDstRegT rd1;
  oneDstRegT rd2;
}bundleT;//hcTracePacketT; 

//typedef hcTracePacketT   gTracePacketT[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_HYPERCONTEXTS_MAX];

typedef struct {
  vtCtrlStateE vt_ctrl;
  bundleT bundle[MASTERCFG_C_ISSUE_WIDTH];
}hcTracePacketT;
typedef hcTracePacketT   gTracePacketT[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_HYPERCONTEXTS_MAX];
/*
  type chcT is record
  c : std_logic_vector(PARSIENlog2(MASTERCFG_CONTEXTS_MAX)-CINT1 downto CINT0);
  hc : std_logic_vector(PARSIENlog2(MASTERCFG_HCONTEXTS_MAX)-CINT1 downto CINT0);
  host : std_logic;
  end record;
*/
typedef struct {
  int c;
  int hc;
  char host;
} chcT;

//  type mutex_stateT is (MUTEX_INVALID, MUTEX_UNLOCKED, MUTEX_LOCKED);
typedef enum {MUTEX_INVALID, MUTEX_UNLOCKED, MUTEX_LOCKED} mutex_stateT;
/*
  type vt_mutexT is record
  -- This is the address of teh mutex, used to uniquely identify it
  addr : std_logic_vector(CINT31 downto CINT0);
  -- creator hchT
  creator : chcT;
  -- initialized?
  initialized : std_logic;
  -- mutex state
  state : mutex_stateT;
  -- lock owner
  lock_owner : chcT;
  end record;
*/
typedef struct {
  int addr;
  chcT creator;
  char initialized;
  mutex_stateT state;
  chcT lock_owner;
} vt_mutexT;

/*
  type vtcu_single_entryT is record
  --
  parent_cpu : chcT;
  mutex_mask : std_logic_vector(0 to VT_MUTEXES_MAX-1);
  --mutex : vt_mutex_array;
  end record;
*/
int postClock(
	      galaxyConfigT *galaxyConfig,
	      //unsigned int system ,
	      //unsigned int context ,
	      //unsigned int hypercontext,
	      gTracePacketT gTracePacket);
typedef struct {
  chcT parent_cpu;
  unsigned int mutex_mask;
} vtcu_single_entryT;
//type vt_mutex_arrayT is array (CINT0 to MASTERCFG_SYSTEMS-CINT1, CINT0 to VT_MUTEXES_MAX-CINT1) of vt_mutexT;
typedef vt_mutexT vt_mutex_arrayT[VT_MUTEXES_MAX];
/*
  type vtcuT is record
  hch_info : vtcu_arrayT;
  mutex_info : vt_mutex_arrayT;
  ctrl_reg : a32_c_hcT;
  end record;
*/
// type vtcu_arrayT is array (CINT0 to VT_SYSTEMS-CINT1 , CINT0 to CONTEXTS-CINT1 , CINT0 to  HYPERCONTEXTS -CINT1) of vtcu_single_entryT;
typedef vtcu_single_entryT vtcu_arrayT[MASTERCFG_CONTEXTS_MAX][MASTERCFG_HYPERCONTEXTS_MAX];
// type a32_c_hcT  is array (CINT0 to VT_SYSTEMS-CINT1, CINT0 to CONTEXTS-CINT1 , CINT0 to  HYPERCONTEXTS -CINT1) of std_logic_vector(CINT31 downto CINT0);
typedef int a32_c_hcT[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_HYPERCONTEXTS_MAX];
typedef struct {
  vtcu_arrayT hch_info;
  vt_mutex_arrayT mutex_info;
  a32_c_hcT ctrl_reg;
} vtcuT;
//  type vtcuAllSystemsT is array (CINT0 to MASTERCFG_SYSTEMS-CINT1) of vtcuT;
typedef vtcuT vtcuAllSystemsT[MASTERCFG_SYSTEMS_MAX];

// type vtcu_sysT is array (CINT0 to CONTEXTS-CINT1 , CINT0 to  HYPERCONTEXTS -CINT1) of vtcu_single_entryT;

//int postClock(galaxyConfigT *,gTracePacketT );
//int matchSyll (int  , char * );

/* These define the binary opcodes of the instructions that are trapped by the vtapi
 *
 */
#define PATTERN_HALT                   "--0010110100001000000000000000"
#define PATTERN_VTHREAD_CREATE_LOCAL   ""
#define PATTERN_VTHREAD_CREATE_REMOTE  ""
#define PATTERN_VTHREAD_JOIN           ""
#define PATTERN_VTHREAD_EXIT           ""
#define PATTERN_VTHREAD_SELF           ""
#define PATTERN_VTHREAD_EQUAL           ""
#define PATTERN_VTHREAD_MUTEX_INIT     "--000101000------000000001------"
#define PATTERN_VTHREAD_MUTEX_LOCK     ""
#define PATTERN_VTHREAD_MUTEX_TRYLOCK  ""
#define PATTERN_VTHREAD_MUTEX_UNLOCK   ""
#define PATTERN_VTHREAD_MUTEX_DESTROY  ""
#define PATTERN_VTHREAD_COND_INIT      ""
#define PATTERN_VTHREAD_COND_DESTROY   ""
#define PATTERN_VTHREAD_COND_SIGNAL    ""
#define PATTERN_VTHREAD_COND_BROADCAST ""
#define PATTERN_VTHREAD_COND_WAIT      ""


int vtApiDbgIfDiags (targetE );
int vtApiDmaTest  (globalTargetT  , int  /*bytes*/);
int vtApiChkGalaxyConfig(globalTargetT , galaxyConfigT ,  galaxyConfigT  );
int vtApiextractStats(statsT* , galaxyConfigT *, hostE  , targetE );
int vtApiPrintStats(galaxyConfigT *, statsT* , targetE );

/*
  type cTracePacketT is array(0 to MASTERCFG_HCONTEXTS_MAX-1) of hcTracePacketT;
  type sTracePacketT is array(0 to MASTERCFG_CONTEXTS_MAX-1) of cTracePacketT;
  type gTracePacketT is array(0 to MASTERCFG_SYSTEMS_MAX-1) of sTracePacketT;
*/
    
//typedef hcTracePacketT  gt[MASTERCFG_SYSTEMS_MAX][MASTERCFG_CONTEXTS_MAX][MASTERCFG_HYPERCONTEXTS_MAX];
#endif /*VTAPI_H_*/
