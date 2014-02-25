#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MSB2LSBDW( x )  (	  \
		( ( x & 0x000000FF ) << 24 ) \
		| ( ( x & 0x0000FF00 ) << 8 ) \
		| ( ( x & 0x00FF0000 ) >> 8 ) \
		| ( ( x & 0xFF000000 ) >> 24 ) \
		)
/*
  Test running Insizzle using API
*/

char *iram = NULL;
char *dram = NULL;
char *machine = NULL;

namespace test {

	extern "C" {
#include "functions.h"
#include "xmlRead.h"
	}

	void usage(char *);
	int checkStatus(void);

	int main(int argc, char *argv[])
	{
		int i;
		/* turn printout on */
		PRINT_OUT = 1;

		for(i=1;i<argc;++i) {
			if(!strncmp(argv[i], "-i", 2)) {
				iram = argv[++i];
			}
			else if(!strncmp(argv[i], "-d", 2)) {
				dram = argv[++i];
			}
			else if(!strncmp(argv[i], "-m", 2)) {
				machine = argv[++i];
			}
		}

		/* Check all options set */
		if(!(iram && dram && machine)) {
			usage(argv[0]);
			exit(-1);
		}

		/* Setup global struct */
		/* readConf reads xml file and sets up global SYSTEM variable
		   SYSTEM is a global pointer to systemConfig defined in inc/galaxyConfig.h
		   This sets up the internal registers of the LE1 as defined in the VTPRM
		*/
		if(readConf(machine) == -1) {
			fprintf(stderr, "Error reading machine model file\n");
			exit(-1);
		}

		/* Configure Insizzle structs */
		/* Using the SYSTEM registers defined above the registers and memories are setup
		   Data structures are defined in inc/galaxy.h
		   galaxyT is a global pointer of type systemT
		*/
		if(setupGalaxy() == -1) {
			fprintf(stderr, "Error setting up galaxy\n");
			exit(-1);
		}

		unsigned int dram_size = 0;
		/* Set the stack pointer and program counter of an available hypercontext */
		{
			extern unsigned int STACK_SIZE; /* global var in Insizzle required for stack checking */

			/* system, context, hypercontext, cluster */
			unsigned char s = 0;
			unsigned char c = 0;
			unsigned char hc = 0;
			unsigned char cl = 0;

			systemConfig *SYS = (systemConfig *)((size_t)SYSTEM + (s * sizeof(systemConfig)));
			systemT *system = (systemT *)((size_t)galaxyT + (s * sizeof(systemT)));

			/* This is a value defined in the xml config file */
			STACK_SIZE = SYS->STACK_SIZE;
			dram_size = ((SYS->DRAM_SHARED_CONFIG >> 8) & 0xFFFF) * 1024;
			printf("dram_size: %d\n", dram_size);

			unsigned int totalHC = 0;
			/* loop through all contexts and hypercontexts */
			for(c=0;c<(SYS->SYSTEM_CONFIG & 0xff);++c){
				contextConfig *CNT = (contextConfig *)((size_t)SYS->CONTEXT + (c * sizeof(contextConfig)));
				contextT *context = (contextT *)((size_t)system->context + (c * sizeof(contextT)));

				for(hc=0;hc<((CNT->CONTEXT_CONFIG >> 4) & 0xf);++hc) {
					hyperContextT *hypercontext = (hyperContextT *)((size_t)context->hypercontext + (hc * sizeof(hyperContextT)));

					/* Set current hypercontext to interact with */
					insizzleAPISetCurrent(s, c, hc, cl); /* (system, context, hypercontext, cluster) */
					/* Write stack pointer (SGPR 1) */
					insizzleAPIWrOneSGpr(1, (dram_size - 256) - ((STACK_SIZE * 1024) * totalHC));
					/* Set original stack pointer and stack size for Insizzle to perform stack check */
					hypercontext->initialStackPointer = (dram_size - 256) - ((STACK_SIZE * 1024) * totalHC);

					/* Write Program Counter */
					insizzleAPIWrPC(0x0);

					++totalHC;
				}
			}
#if 0
			s = c = hc = cl = 0;

			/* Set current hypercontext to interact with */
			insizzleAPISetCurrent(s, c, hc, cl); /* (system, context, hypercontext, cluster) */
			/* Write stack pointer (SGpr 1) */
			insizzleAPIWrOneSGpr(1, (dram_size - 256) - (STACK_SIZE * 1024));
			/* Set original stack pointer and stack size for Insizzle to perform stack check */
			{
				systemT *system = (systemT *)((size_t)galaxyT + (s * sizeof(systemT)));
				contextT *context = (contextT *)((size_t)system->context + (c * sizeof(contextT)));
				hyperContextT *hypercontext = (hyperContextT *)((size_t)context->hypercontext + (hc * sizeof(hyperContextT)));
				unsigned int stack_pointer;
				insizzleAPIRdOneSGpr(1, &stack_pointer);
				hypercontext->initialStackPointer = stack_pointer;
			}
			/* Write Program Counter */
			insizzleAPIWrPC(0x0);
#endif
		}

		/* Load IRAM */
		{
			unsigned int fileSize = 0;
			char *i_data = NULL;

			FILE *inst = fopen(iram, "rb");
			if(inst == NULL) {
				fprintf(stderr, "Could not open file (%s)\n", iram);
				exit(-1);
			}
			fseek(inst, 0L, SEEK_END);
			fileSize = ftell(inst);
			fseek(inst, 0L, SEEK_SET);

			/* Create local data and copy content of file into it */
			i_data = (char *)calloc(sizeof(char), fileSize);
			if(i_data == NULL) {
				fprintf(stderr, "Could not allocate memory (i_data)\n");
				exit(-1);
			}
			fread(i_data, sizeof(char), fileSize, inst);

			/* Input into each available iram */
			{
				/* system, context, hypercontext, cluster */
				unsigned char s = 0;
				unsigned char c = 0;
				unsigned char hc = 0;
				unsigned char cl = 0;

				systemConfig *SYS = (systemConfig *)((size_t)SYSTEM + (s * sizeof(systemConfig)));

				/* loop through all contexts and hypercontexts */
				for(c=0;c<(SYS->SYSTEM_CONFIG & 0xff);++c){
					contextConfig *CNT = (contextConfig *)((size_t)SYS->CONTEXT + (c * sizeof(contextConfig)));

					for(hc=0;hc<((CNT->CONTEXT_CONFIG >> 4) & 0xf);++hc) {
						/* Set current hypercontext to interact with */
						insizzleAPISetCurrent(s, c, hc, cl); /* (system, context, hypercontext, cluster) */
						/* Load Iram into current hypercontext */
						insizzleAPILdIRAM(i_data, fileSize);

						{
							/* Check correct */
							unsigned int i;
							unsigned int word;
							for(i=0;i<fileSize;i+=4) {
								insizzleAPIRdOneIramLocation(i, &word);
								/* Need to perform an endian flip */
								if(MSB2LSBDW(word) != (unsigned int)*(unsigned int *)(i_data + i)) {
									fprintf(stderr, "error: 0x%08x != 0x%08x\n",
									        word,
									        (unsigned int)*(unsigned int *)(i_data + i));
									exit(-1);
								}
							}
						}
					}
				}
			}

			/* Free memory */
			free(i_data);
		}

		/* Load DRAM */
		{
			unsigned int fileSize = 0;
			char *d_data = NULL;

			FILE *data = fopen(dram, "rb");
			if(data == NULL) {
				fprintf(stderr, "Could not open file (%s)\n", dram);
				exit(-1);
			}
			fseek(data, 0L, SEEK_END);
			fileSize = ftell(data);
			fseek(data, 0L, SEEK_SET);

			if(fileSize > dram_size) {
				fprintf(stderr, "DRAM is larger than the size specified.\n");
				exit(-1);
			}

			/* Create local data and copy content of file into it */
			d_data = (char *)calloc(sizeof(char), dram_size);
			if(d_data == NULL) {
				fprintf(stderr, "Could not allocate memory (d_data)\n");
				exit(-1);
			}
			fread(d_data, sizeof(char), fileSize, data);

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
					fprintf(stderr, "error: 0x%08x != 0x%08x\n",
					        word,
					        (unsigned int)*(unsigned int *)(d_data + i));
					exit(-1);
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
			/* system, context, hypercontext, cluster */
			unsigned char s = 0;
			unsigned char c = 0;
			unsigned char hc = 0;
			unsigned char cl = 0;

			systemConfig *SYS = (systemConfig *)((size_t)SYSTEM + (s * sizeof(systemConfig)));

			/* loop through all contexts and hypercontexts */
			for(c=0;c<(SYS->SYSTEM_CONFIG & 0xff);++c){
				contextConfig *CNT = (contextConfig *)((size_t)SYS->CONTEXT + (c * sizeof(contextConfig)));

				for(hc=0;hc<((CNT->CONTEXT_CONFIG >> 4) & 0xf);++hc) {
					/* Set current hypercontext to interact with */
					insizzleAPISetCurrent(s, c, hc, cl); /* (system, context, hypercontext, cluster) */

					/* vtCtrlStatE enum defined in vtapi.h */
					insizzleAPIWrCtrl(RUNNING);
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

			systemConfig *SYS = (systemConfig *)((size_t)SYSTEM + (s * sizeof(systemConfig)));

			/* loop through all contexts and hypercontexts */
			for(c=0;c<(SYS->SYSTEM_CONFIG & 0xff);++c){
				contextConfig *CNT = (contextConfig *)((size_t)SYS->CONTEXT + (c * sizeof(contextConfig)));

				for(hc=0;hc<((CNT->CONTEXT_CONFIG >> 4) & 0xf);++hc) {
					unsigned int val;

					/* Set current hypercontext to interact with */
					insizzleAPISetCurrent(s, c, hc, cl); /* (system, context, hypercontext, cluster) */

					insizzleAPIRdCtrl(&val);
					printf("vt_ctrl: %d\n", val);
				}
			}
		}



		extern unsigned long long cycleCount;
		/* Clock */
		{
			unsigned int vt_ctrl;
			cycleCount = 0;
			while(checkStatus()) {
				galaxyConfigT *g = NULL;
				gTracePacketT gTracePacket;
				memset((void *)&gTracePacket, 0, sizeof(gTracePacket)); /* clear gTracePacket memory */
				printf("------------------------------------------------------------ end of cycle %lld\n", cycleCount);
				insizzleAPIClock(g, gTracePacket);

				/* Check here for control flow changes */
				/* Nothing in gTracePacket to find this */
				/* (inst.packet.newPCValid available in Insizzle for this) */

				insizzleAPIRdCtrl(&vt_ctrl);
				++cycleCount;
			}
		}

		/* Clean up */
		if(freeMem() == -1) {
			fprintf(stderr, "Error freeing memory\n");
			exit(-1);
		}
		return 0;
	}

	void usage(char *prog)
	{
		fprintf(stderr, "Usage:\n");
		fprintf(stderr, "%s -i <iram_file> -d <dram_file> -m <xml_machine_model>\n", prog);
		return;
	}

/* Loop through available hypercontexts checking VT_CTRL register */
	int checkStatus(void)
	{
		/* system, context, hypercontext, cluster */
		unsigned char s = 0;
		unsigned char c = 0;
		unsigned char hc = 0;
		unsigned char cl = 0;

		systemConfig *SYS = (systemConfig *)((size_t)SYSTEM + (s * sizeof(systemConfig)));

		/* loop through all contexts and hypercontexts */
		for(c=0;c<(SYS->SYSTEM_CONFIG & 0xff);++c){
			contextConfig *CNT = (contextConfig *)((size_t)SYS->CONTEXT + (c * sizeof(contextConfig)));

			for(hc=0;hc<((CNT->CONTEXT_CONFIG >> 4) & 0xf);++hc) {
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
};

int main(int argc, char *argv[])
{
	return test::main(argc, argv);
}
