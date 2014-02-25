#ifndef _GUARDMACROS
#define _GUARDMACROS
/* instruction macros
   borrows from the VEX .cs.c file
   will need altering though
 */

/***********************************************/
/*         MACHINE MODEL HEADER                */
/***********************************************/

#define UINT8(s)  ((s) & 0xff)
#define INT8(s)   (((int) ((s) << 24)) >> 24)
#define UINT16(s) ((s) & 0xffff)
#define INT16(s)  (((int) ((s) << 16)) >> 16)
#define UINT32(s) ((unsigned int) (s))
#define INT32(s)  ((int) (s))


 		 /*  MEMORY MACROS */

#define ADDR8(a)  ((a) ^ 0x3)
#define ADDR16(a) ((a) ^ 0x2)
#define ADDR32(a) (a)
#define MEM8(a) (*((volatile unsigned char  *)(ADDR8(a))))
#define MEM16(a) (*((volatile unsigned short *)(ADDR16(a))))
#define MEM32(a) (*((volatile unsigned int *)(ADDR32(a))))
#define MEMSPEC8(a) sim_mem_access8(ADDR8(a))
#define MEMSPEC16(a) sim_mem_access16(ADDR16(a))
#define MEMSPEC32(a) sim_mem_access32(ADDR32(a))
#define LDBs(t,s1) t = INT8(MEMSPEC8(s1))
#define LDB(t,s1) t = INT8(MEM8(s1))
#define LDBUs(t,s1) t = UINT8(MEMSPEC8(s1))
#define LDBU(t,s1) t = UINT8(MEM8(s1))
#define LDHs(t,s1) t = INT16(MEMSPEC16(s1))
#define LDH(t,s1) t = INT16(MEM16(s1))
#define LDHUs(t,s1) t = UINT16(MEMSPEC16(s1))
#define LDHU(t,s1) t = UINT16(MEM16(s1))
#define LDWs(t,s1) t = INT32(MEMSPEC32(s1))
#define LDW(t,s1) t = INT32(MEM32(s1))
#define STB(t,s1) MEM8(t) = UINT8(s1)
#define STH(t,s1) MEM16(t) = UINT16(s1)
#define STW(t,s1) MEM32(t) = UINT32(s1)


 		 /*  INSTRUCTION MACROS */

#define MLSL(t,s1,s2) t = (s1) * INT16(s2)
#define MLUL(t,s1,s2) t = (s1) * UINT16(s2)
#define MLSH(t,s1,s2) t = (s1) * INT16((s2) >> 16)
#define MLUH(t,s1,s2) t = (s1) * UINT16((s2) >> 16)
#define MLSHS(t,s1,s2) t = ((s1) * UINT16((s2) >> 16)) << 16
#define MLSLL(t,s1,s2)  t = INT16(s1) * INT16(s2)
#define MLULL(t,s1,s2) t = UINT16(s1) * UINT16(s2)
#define MLSLH(t,s1,s2)  t = INT16(s1) * INT16((s2) >> 16)
#define MLULH(t,s1,s2) t = UINT16(s1) * UINT16((s2) >> 16)
#define MLSHH(t,s1,s2)  t = INT16((s1) >> 16) * INT16((s2) >> 16)
#define MLUHH(t,s1,s2) t = UINT16((s1) >> 16) * UINT16((s2) >> 16)

#define ADD(t,s1,s2) t = (s1) + (s2)
#define AND(t,s1,s2) t = (s1) & (s2)
#define ANDC(t,s1,s2) t = ~(s1) & (s2)
#define ANDL(t,s1,s2) t = ((((s1) == 0) | ((s2) == 0)) ? 0 : 1)
#define CMPEQ(t,s1,s2) t = ((s1) == (s2))
#define CMPGES(t,s1,s2) t = ((int) (s1) >= (int) (s2))
#define CMPGEU(t,s1,s2) t = ((unsigned int) (s1) >= (unsigned int) (s2))
#define CMPGTS(t,s1,s2) t = ((int) (s1) > (int) (s2))
#define CMPGTU(t,s1,s2) t = ((unsigned int) (s1) > (unsigned int) (s2))
#define CMPLES(t,s1,s2) t = ((int) (s1) <= (int) (s2))
#define CMPLEU(t,s1,s2) t = ((unsigned int) (s1) <= (unsigned int) (s2))
#define CMPLTS(t,s1,s2) t = ((int) (s1) < (int) (s2))
#define CMPLTU(t,s1,s2) t = ((unsigned int) (s1) < (unsigned int) (s2))
#define CMPNE(t,s1,s2) t = ( (s1) !=  (s2))
#define MAXS(t,s1,s2) t = ((int) (s1) > (int) (s2)) ? (s1) : (s2)
#define MAXU(t,s1,s2) t = ((unsigned int) (s1) > (unsigned int) (s2)) ? (s1) : (s2)
#define MINS(t,s1,s2) t = ((int) (s1) < (int) (s2)) ? (s1) : (s2)
#define MINU(t,s1,s2) t = ((unsigned int) (s1) < (unsigned int) (s2)) ? (s1) : (s2)
#define MFB(t,s1) t = s1
#define MFL(t,s1) t = s1
#define MOV(t,s1) t = s1
#define MTL(t,s1) t = s1
#define MTB(t,s1) t = ((s1) == 0) ? 0 : 1
#define MTBF(t,s1) t = ((s1) == 0) ? 1 : 0
#define MPY(t,s1,s2) t = (s1) * (s2)
#define NANDL(t,s1,s2) t = (((s1) == 0) | ((s2) == 0)) ? 1 : 0

#define NORL(t,s1,s2) t = (((s1) == 0) & ((s2) == 0)) ? 1 : 0
#define ORL(t,s1,s2) t = (((s1) == 0) & ((s2) == 0)) ? 0 : 1
#define OR(t,s1,s2) t = (s1) | (s2)
#define ORC(t,s1,s2) t = (~(s1)) | (s2)
#define PFT(s1) (s1)
#define SBIT(t,s1,s2) t = (s1) | ((unsigned int) 1 << (s2))
#define SBITF(t,s1,s2) t = (s1) & ~((unsigned int) 1 << (s2))
#define SH1ADD(t,s1,s2) t = ((s1) << 1) + (s2)
#define SH2ADD(t,s1,s2) t = ((s1) << 2) + (s2)
#define SH3ADD(t,s1,s2) t = ((s1) << 3) + (s2)
#define SH4ADD(t,s1,s2) t = ((s1) << 4) + (s2)
#define SHL(t,s1,s2) t = ((int) (s1)) << (s2)
#define SHRS(t,s1,s2) t = ((int) (s1)) >> (s2)
#define SHRU(t,s1,s2) t = ((unsigned int) (s1)) >> (s2)
#define SWBT(t,s1,s2,s3) t = (unsigned int) (((s1) != 0) ? (s2) : (s3))
#define SWBF(t,s1,s2,s3) t = (unsigned int) (((s1) == 0) ? (s2) : (s3))
#define SUB(t,s1,s2) t = (s1) - (s2)
/* reverse sub */
#define RSUB(t,s1,s2) t = (s1) - (s2)
#define SEXTB(t,s1) t = (unsigned int) (((signed int) ((s1) << 24)) >> 24)
#define SEXTH(t,s1) t = (unsigned int) (((signed int) ((s1) << 16)) >> 16)
#define TBIT(t,s1,s2) t = ((s1) & ((unsigned int) 1 << (s2))) ? 1 : 0
#define TBITF(t,s1,s2) t = ((s1) & ((unsigned int) 1 << (s2))) ? 0 : 1
#define XNOP(s1) printf("XNOP: %d\n", s1)
#define XOR(t,s1,s2) t = (s1) ^ (s2)
#define ZEXTB(t,s1) t = ((s1) & 0xff)
#define ZEXTH(t,s1) t = ((s1) & 0xffff)
#define ADDCG(t,cout,s1,s2,cin) {\
    t = (s1) + (s2) + ((cin) & 0x1);\
    cout =   ((cin) & 0x1)\
           ? ((unsigned int) t <= (unsigned int) (s1))\
           : ((unsigned int) t <  (unsigned int) (s1));\
}
#define DIVS(t,cout,s1,s2,cin) {\
    unsigned int tmp = ((s1) << 1) | (cin);\
    cout = (unsigned int) (s1) >> 31;\
    t = cout ? tmp + (s2) : tmp - (s2);\
}


/* control flows */
/* at the moment don't do anything with them */
/* they are dealt with at the end of the cycle */
/*#define CALL()
#define GOTO()
#define BR()
#define BRF()
#define RETURN(w,x,y,z)*/


	/* own load/store stuff */
/* called from the memory check stage in driver thread */
#define _LDUB_iss(t,s1)	  \
	do { \
		unsigned int* addr; \
		addr = (unsigned int*)((((size_t)s1) - (((size_t)s1) % 4)) + (3 - (((size_t)s1) % 4))); \
		*(t) = _UINT8_iss(*((unsigned char*)(addr)));		\
	} while(0)

#define _LDSB_iss(t,s1)	  \
	do { \
		unsigned int* addr; \
		addr = (unsigned int*)((((size_t)s1) - (((size_t)s1) % 4)) + (3 - (((size_t)s1) % 4))); \
		*(t) = _INT8_iss(*((unsigned char*)(addr)));		\
	} while(0)

#define _LDBs_iss(t,s1)	  \
	do { \
		unsigned int* addr; \
		addr = (unsigned int*)((((size_t)s1) - (((size_t)s1) % 4)) + (3 - (((size_t)s1) % 4))); \
		*(t) = _UINT8_iss(*((unsigned char*)(addr)));		\
	} while(0)

#define _LDUH_iss(t,s1)	  \
	do { \
		unsigned int* addr; \
		addr = (unsigned int*)((((size_t)s1) - (((size_t)s1) % 4)) + (2 - (((size_t)s1) % 4))); \
		*(t) = _UINT16_iss(*((unsigned short*)(addr)));		\
	} while(0)

#define _LDSH_iss(t,s1)	  \
	do { \
		unsigned int* addr; \
		addr = (unsigned int*)((((size_t)s1) - (((size_t)s1) % 4)) + (2 - (((size_t)s1) % 4))); \
		*(t) = _INT16_iss(*((unsigned short*)(addr)));		\
	} while(0)

#define _LDW_iss(t,s1)	  \
	do { \
	  *(t) = _INT32_iss(*((unsigned int*)(s1)));	\
	} while(0)


#define _STB_iss(t,s1)							\
  do {									\
    unsigned int* addr;							\
    addr = (unsigned int*)((((size_t)t) - (((size_t)t) % 4)) + (3 - (((size_t)t) % 4))); \
    *((unsigned char*)(addr)) = (unsigned char)_UINT8_iss(*s1);		\
  } while(0)

#define _STH_iss(t,s1)							\
  do {									\
    unsigned int* addr;							\
    addr = (unsigned int*)((((size_t)t) - (((size_t)t) % 4)) + (2 - (((size_t)t) % 4))); \
    *((unsigned short*)(addr)) = (unsigned short)_UINT16_iss(*s1);	\
  } while(0)

#define _STW_iss(t,s1)							\
  do {									\
    *((unsigned int*)(t)) = (unsigned int)_UINT32_iss(*s1);		\
  } while(0)

#define _UINT8_iss(s)  ((s) & 0xff)
#define _INT8_iss(s)   (((int) ((s) << 24)) >> 24)
#define _UINT16_iss(s) ((s) & 0xffff)
#define _INT16_iss(s)  (((int) ((s) << 16)) >> 16)
#define _UINT32_iss(s) ((unsigned int) (s))
#define _INT32_iss(s)  ((int) (s))

/* called from the simulator */
/* need to add out of bounds check */
#define LDSB_iss(t,s1,cnt,hcnt)						\
  do {									\
    if((s1 - (unsigned int)data) > STACK_SIZE)				\
      {									\
	t = 0;								\
	printf("ERROR (LDSB) (0x%x)\n", (s1 - (unsigned int)data));	\
      }									\
    else								\
      {									\
	_LDSB_iss(t,s1);						\
      }									\
    newMemRequest((((s1 - (unsigned int)data) >> 2) & findMemBlocks), ((cnt << 24) | (hcnt << 20))); \
  } while(0)

#define LDBs_iss(t,s1,cnt,hcnt)						\
  do {									\
    if((s1 - (unsigned int)data) > STACK_SIZE)				\
      {									\
	t = 0;								\
	printf("ERROR (LBDs) (0x%x)\n", (s1 - (unsigned int)data))	\
	  }								\
    else								\
      {									\
	_LDBs_iss(t,s1);						\
      }									\
    newMemRequest((((s1 - (unsigned int)data) >> 2) & findMemBlocks), ((cnt << 24) | (hcnt << 20))); \
  } while(0)

#define LDUB_iss(t,s1,cnt,hcnt)						\
  do {									\
    if((s1 - (unsigned int)data) > STACK_SIZE)				\
      {									\
	t = 0;								\
	printf("ERROR (LDUB) (0x%x)\n", (s1 - (unsigned int)data));	\
      }									\
    else								\
      {									\
	_LDUB_iss(t,s1);						\
      }									\
    newMemRequest((((s1 - (unsigned int)data) >> 2) & findMemBlocks), ((cnt << 24) | (hcnt << 20))); \
  } while(0)

#define LDSH_iss(t,s1,cnt,hcnt)						\
  do {									\
    if((s1 - (unsigned int)data) > STACK_SIZE)				\
      {									\
	t = 0;								\
	printf("ERROR (LDSH) (0x%x)\n", (s1 - (unsigned int)data));	\
      }									\
    else								\
      {									\
	_LDSH_iss(t,s1);						\
      }									\
    newMemRequest((((s1 - (unsigned int)data) >> 2) & findMemBlocks), ((cnt << 24) | (hcnt << 20))); \
  } while(0)

#define LDUH_iss(t,s1,cnt,hcnt)						\
  do {									\
    if((s1 - (unsigned int)data) > STACK_SIZE)				\
      {									\
	t = 0;								\
	printf("ERROR (LDUH) (0x%x)\n", (s1 - (unsigned int)data));	\
      }									\
    else								\
      {									\
	_LDUH_iss(t,s1);						\
      }									\
    newMemRequest((((s1 - (unsigned int)data) >> 2) & findMemBlocks), ((cnt << 24) | (hcnt << 20))); \
  } while(0)

#define LDW_iss(t,s1,cnt,hcnt)						\
  do {									\
    if((s1 - (unsigned int)data) > STACK_SIZE)				\
      {									\
	t = 0;								\
	printf("ERROR (LDW) (0x%x)\n", (s1 - (unsigned int)data));	\
      }									\
    else								\
      {									\
	_LDW_iss(t,s1);							\
      }									\
    newMemRequest((((s1 - (unsigned int)data) >> 2) & findMemBlocks), ((cnt << 24) | (hcnt << 20))); \
  } while(0)

#define STB_iss(t,s1,cnt,hcnt)						\
  do {									\
    if((t - (unsigned int)data) > STACK_SIZE)				\
      {									\
	/* error: trying to store to no memory*/			\
	printf("ERROR (STB) (0x%x)\n", (t - (unsigned int)data));	\
      }									\
    else								\
      {									\
	_STB_iss(t,s1);							\
      }									\
    newMemRequest((((t - (unsigned int)data) >> 2) & findMemBlocks), ((cnt << 24) | (hcnt << 20))); \
  } while(0)

#define STBs_iss(t,s1,cnt,hcnt)						\
  do {									\
    if((t - (unsigned int)data) > STACK_SIZE)				\
      {									\
	/* error: trying to store to no memory*/			\
	printf("ERROR (STBs) (0x%x)\n", (t - (unsigned int)data));	\
      }									\
    else								\
      {									\
	_STBs_iss(t,s1);						\
      }									\
    newMemRequest((((t - (unsigned int)data) >> 2) & findMemBlocks), ((cnt << 24) | (hcnt << 20))); \
  } while(0)

#define STH_iss(t,s1,cnt,hcnt)						\
  do {									\
    if((t - (unsigned int)data) > STACK_SIZE)				\
      {									\
	/* error: trying to store to no memory*/			\
	printf("ERROR (STH) (0x%x)\n", (t - (unsigned int)data));	\
      }									\
    else								\
      {									\
	_STH_iss(t,s1);							\
      }									\
    newMemRequest((((t - (unsigned int)data) >> 2) & findMemBlocks), ((cnt << 24) | (hcnt << 20))); \
  } while(0)

#define STW_iss(t,s1,cnt,hcnt)						\
  do {									\
    if((t - (unsigned int)data) > STACK_SIZE)				\
      {									\
	/* error: trying to store to no memory*/			\
	printf("ERROR (STW) (0x%x)\n", (t - (unsigned int)data));	\
      }									\
    else								\
      {									\
	_STW_iss(t,s1);							\
      }									\
    newMemRequest((((t - (unsigned int)data) >> 2) & findMemBlocks), ((cnt << 24) | (hcnt << 20))); \
  } while(0)

#endif
