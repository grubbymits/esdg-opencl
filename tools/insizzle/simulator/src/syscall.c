#include "syscall.h"
#include "macros.h"
#include "functions.h"
#include "pcre_split.h"
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define PRINTF     0
#define FOPEN      1
#define FCLOSE     2
#define FWRITE     3
#define FREAD      4
#define FPRINTF    5
#define FSEEK      6
#define FTELL      7
#define SNPRINTF   8
#define CLOCK      9
#define FSCANF    10
#define FGETC     11
#define UNGETC    12
#define ATOI      13
#define SRAND     14
#define RAND      15
#define SPRINTF   16
#define SSCANF    17
#define FPUTC     18
#define OPEN      19
#define CLOSE     20
#define READ      21
#define WRITE     22
#define LSEEK     23
#define FSTAT     24
#define STRTOL    25
#define STRTOD    26

#define OUT stderr
/* hacky way around 64 -> 32 bit file pointers */
typedef struct {
  FILE *file;
  char valid;
} filepointer_t;

filepointer_t filepointer[5] = {{NULL, 0}, {NULL, 0}, {NULL, 0}, {NULL, 0}, {NULL, 0}};

unsigned char isLLVM;
time_t start, end;

/* find a free space in the filepointer array */
int setFilePointer(FILE *fp) {
  int i;
  for(i=0;i<5;i++) {
    if(!(filepointer[i].valid)) {
      filepointer[i].file = fp;
      filepointer[i].valid = 1;
      return i;
    }
  }
  fprintf(stderr, "No free space for file pointer\n");
  return -1;
}
/* retreive a filepointer from array */
FILE *getFilePointer(int i) {
  if(filepointer[i-1].valid) {
    return filepointer[i-1].file;
  }
  return NULL;
}
/* free a filepointer space */
void freeFilePointer(int i) {
  filepointer[i-1].file = NULL;
  filepointer[i-1].valid = 0;
}

void _syscall(unsigned *S_GPR, systemT *system, unsigned call, unsigned long long cycleCount) {
  unsigned long dram = (size_t)system->dram;

  if(isLLVM) {
    /* check whether there are any memory operations waiting on the registers required */
    /* max number of registers is 7, create a local copy */
    unsigned _reg[10];
    int i;
    struct memReqT *temp;
    int found = 0;

    for(i=0;i<10;i++) {
      temp = system->memReq;
      found = 0;
      if(temp != NULL) {
	do {
	  if((size_t)temp->pointer == (size_t)(S_GPR + i)) {
	    /* need to get it from memory (only loads) */
	    found = 1;
	    switch(temp->memOp) {
	    case mLDSB:
	      _LDSB_iss(&_reg[i], (temp->value + (size_t)(system->dram)));
	      break;
	    case mLDBs:
	      _LDBs_iss(&_reg[i], (temp->value + (size_t)(system->dram)));
	      break;
	    case mLDUB:
	      _LDUB_iss(&_reg[i], (temp->value + (size_t)(system->dram)));
	      break;
	    case mLDSH:
	      _LDSH_iss(&_reg[i], (temp->value + (size_t)(system->dram)));
	      break;
	    case mLDUH:
	      _LDUH_iss(&_reg[i], (temp->value + (size_t)(system->dram)));
	      break;
	    case mLDW:
	      _LDW_iss(&_reg[i], (temp->value + (size_t)(system->dram)));
	      break;
	    default:
	      /* ignore i guess */
	      found = 0;
	      break;
	    }
	    break;
	  }
	  temp = temp->next;
	} while(temp != NULL);
	if(!found) {
	  _reg[i] = (unsigned)*(S_GPR + i);
	}
      }
      else {
	_reg[i] = (unsigned)*(S_GPR + i);
      }
    }

    /* set S_GPR pointer to our local registers */
    S_GPR = &_reg[0];
  }

  switch(call) {
    /* All assume registers are availabe in current
       registers, due to VEX seeming to work this way
    */
  case PRINTF:
    /* r3 = printf(r3, r4...r10);*/
    {
      char format[256] = {'\0'};
      getString(&format[0], 256, (*(S_GPR + 3) + dram));
      /* PCRE section */
      split_t *_test;
      split_t *_temp;
      int _count = 4; /* arguments start from register 4 */

      _test = pcre_split("%[\\+#-]?(\\d+|\\*)?\\.?\\d*([hlLzjt]|[hl]{2})?([csuxXfFeEpgGdionz])",format);
      _temp = _test;
      if(_test == NULL)
	printf("Error: Nothing to print\n");
      else {
	do {
	  if(_test->string != NULL)
	    fprintf(OUT, "%s", _test->string);
			      
	  if(_test->match != NULL) {
	    switch(*(char *)((size_t)_test->match + strlen(_test->match) - 1)) {
	    case 's':
	    case 'S':
	      {
		char str[256] = {'\0'};
		/* TODO: need to check that this is within memory */
		/* causing segfault because string is within the register :S */
		getString(&str[0], 256, *(S_GPR + _count++) + dram);
		fprintf(OUT, _test->match, str);
	      }
	    break;
	    case 'f':
	    case 'F':
	      {
		/* first register needs to be odd :S */
		if(!(_count % 2)) { _count++; } /* skip odd register */
		double d;
		int *dP = (int *)&d;
		*(dP+1) = *(S_GPR + _count++);
		*dP = *(S_GPR + _count++);
		fprintf(OUT, _test->match, d);
	      }
	    break;
	    default:
	      /* TODO: may need to perform an endian swap? */
	      {
#if 0
		int temp = *(S_GPR + _count++);
		if(!(temp >> 8)) {
		  /* possibly char */
		  printf(_test->match, temp);
		}
		else if(!(temp >> 16)) {
		  /* possibly short */
		  printf(_test->match, ntohs(temp));
		}
		else {
		  /* possibly int */
		  printf(_test->match, ntohl(temp));
		}
#endif
		fprintf(OUT, _test->match, *(S_GPR + _count++));
	      }
	      break;
	    }
	  }
	  if(_count > 10) {
	    fprintf(OUT, "*** Limit of Insizzle printf ***\n");
	    break;
	  }
	} while((_test = _test->next) != NULL);
      }
			  
      /* TODO: fix this */
      /*pcre_split_free(_temp);*/
    }
    break;
  case FOPEN:
    /* r3 = fopen(r3, r4) */
    {
      char filename[256] = {'\0'};
      getString(&filename[0], 256, *(S_GPR + 3) + dram);
      char filemode[256] = {'\0'};
      getString(&filemode[0], 256, *(S_GPR + 4) + dram);

      /*printf("filename: %s\n", filename);
	printf("filemode: %s\n", filemode);*/

      FILE *fp = fopen(filename, filemode);
      int _fp = setFilePointer(fp);
      if(_fp != -1) {
	*(S_GPR + 3) = (unsigned)_fp + 1;
      }
      else {
	*(S_GPR + 3) = 0;
      }
    }
    break;
  case FCLOSE:
    /* r3 = fclose(r3) */
    {
      /* TODO: this will break in 64 bit mode */
      FILE *stream = getFilePointer(*(S_GPR + 3));

      int ret = fclose(stream);

      *(S_GPR + 3) = (unsigned)ret;
    }
    break;
  case FWRITE:
    /* r3 = fwrite(r3, r4, r5, r6) */
    {
      void *ptr = (void *)(*(S_GPR + 3) + dram);
      size_t size = (size_t)*(S_GPR + 4);
      size_t nitems = (size_t)*(S_GPR + 5);
      FILE *stream = getFilePointer(*(S_GPR + 6));

      char *ptrLittle = (char *)calloc((size * nitems), 1);
      endianSwapBig2Little(ptrLittle, size, nitems, ptr);
      size_t ret = fwrite(ptrLittle, size, nitems, stream);
      *(S_GPR + 3) = (unsigned)ret;
			  
    }
    break;
  case FREAD:
    /* r3 = fread(r3, r4, r5, r6) */
    {
      void *ptr = (void *)(*(S_GPR + 3) + dram);
      size_t size = (size_t)*(S_GPR + 4);
      size_t nitems = (size_t)*(S_GPR + 5);
      FILE *stream = getFilePointer(*(S_GPR + 6));
			  
      /* copy into local memory, then perform endian switch into LE1 space */
      char *ptrLittle = (char *)calloc((size * nitems), 1);
      size_t ret = fread(ptrLittle, size, nitems, stream);
      endianSwapLittle2Big(ptr, size, nitems, ptrLittle);
      *(S_GPR + 3) = (unsigned)ret;
    }
    break;
  case FPRINTF:
    /* r3 = fprintf(r3, r4, r5...r10) */
    {
      FILE *stream = getFilePointer(*(S_GPR + 3));
      char format[256] = {'\0'};
      getString(&format[0], 256, *(S_GPR + 4) + dram);

      /* PCRE section */
      split_t *_test;
      split_t *_temp;
      int _count = 5; /* arguments start from register 4 */

      _test = pcre_split("%[\\+#-]?(\\d+|\\*)?\\.?\\d*([hlLzjt]|[hl]{2})?([csuxXfFeEpgGdionz])",format);
      _temp = _test;
      if(_test == NULL)
	printf("Error: Nothing to print\n");
      else {
	do {
	  if(_test->string != NULL)
	    fprintf(stream, "%s", _test->string);
			      
	  if(_test->match != NULL) {
	    switch(*(char *)((size_t)_test->match + strlen(_test->match) - 1)) {
	    case 's':
	    case 'S':
	      {
		char str[256] = {'\0'};
		/* TODO: need to check that this is within memory */
		/* causing segfault because string is within the register :S */
		getString(&str[0], 256, *(S_GPR + _count++) + dram);
		fprintf(stream, _test->match, str);
	      }
	    break;
	    case 'f':
	    case 'F':
	      {
		/* first register needs to be odd :S */
		if(!(_count % 2)) { _count++; } /* skip odd register */
		double d;
		int *dP = (int *)&d;
		*(dP+1) = *(S_GPR + _count++);
		*dP = *(S_GPR + _count++);
		fprintf(stream, _test->match, d);
	      }
	    break;
	    default:
	      /* TODO: may need to perform an endian swap? */
	      {
		fprintf(stream, _test->match, *(S_GPR + _count++));
	      }
	      break;
	    }
	  }
	  if(_count > 10) {
	    fprintf(stream, "*** Limit of Insizzle printf ***\n");
	    break;
	  }
	} while((_test = _test->next) != NULL);
      }
      /* TODO: work out freeing this without stack dump */
      /* if there are no extra parts it breaks */
      /*pcre_split_free(_temp);*/
    }
    break;
  case FSEEK:
    /* r3 = fseek(r3, r4, r5) */
    {
      FILE *stream = getFilePointer(*(S_GPR + 3));
      long offset = (long)*(S_GPR + 4);
      int whence = (int)*(S_GPR + 5);

      int ret = fseek(stream, offset, whence);

      *(S_GPR + 3) = (unsigned)ret;
    }
    break;
  case FTELL:
    /* r3 = ftell(r3) */
    {
      FILE *stream = getFilePointer(*(S_GPR + 3));

      long ret = ftell(stream);

      *(S_GPR + 3) = (unsigned)ret;
    }
    break;
  case SNPRINTF:
    /* r3 = snprintf(r3, r4, r5, ...) */
    {
      char *s = (char *)(*(S_GPR + 3) + dram);
      size_t n = (size_t)*(S_GPR + 4);
      char format[256] = {'\0'};
      getString(&format[0], 256, *(S_GPR + 5) + dram);


      /* make a char array the size of n */
      char *sLittle = calloc(n+1, 1);

      /* PCRE section */
      split_t *_test;
      split_t *_temp;
      int _count = 6; /* arguments start from register 4 */

      _test = pcre_split("%[\\+#-]?(\\d+|\\*)?\\.?\\d*([hlLzjt]|[hl]{2})?([csuxXfFeEpgGdionz])",format);
      _temp = _test;
      if(_test == NULL)
	printf("Error: Nothing to print\n");
      else {
	do {
	  if(_test->string != NULL)
	    strcat(sLittle, _test->string);
			      
	  if(_test->match != NULL) {
	    switch(*(char *)((size_t)_test->match + strlen(_test->match) - 1)) {
	    case 's':
	    case 'S':
	      {
		char str[256] = {'\0'};
		getString(&str[0], 256, *(S_GPR + _count++) + dram);
		char _s[256];
		snprintf(_s, 256, _test->match, str);
		strcat(sLittle, _s);
	      }
	    break;
	    case 'f':
	    case 'F':
	      {
		/* first register needs to be odd :S */
		if(!(_count % 2)) { _count++; } /* skip odd register */
		double d;
		int *dP = (int *)&d;
		*(dP+1) = *(S_GPR + _count++);
		*dP = *(S_GPR + _count++);

		char _s[256];
		snprintf(_s, 256, _test->match, d);
		strcat(sLittle, _s);
	      }
	    break;
	    default:
	      {
		char _s[256];
		snprintf(_s, 256, _test->match, *(S_GPR + _count++));
		strcat(sLittle, _s);
	      }
	      break;
	    }
	  }
	  if(_count > 10) {
	    printf("*** Limit of Insizzle printf ***\n");
	    break;
	  }
	} while((_test = _test->next) != NULL);
      }
			  
      /* TODO: fix this */
      /*pcre_split_free(_temp);*/

      /* need to copy sLittle into s */
      endianSwapLittle2Big(s, 1, strlen(sLittle), sLittle);
    }
    break;
  case CLOCK:
    {
      *(S_GPR + 3) = (unsigned)cycleCount;
    }
    break;
  case FSCANF:
    /* r3 = fscanf(r3, r4, ...) */
    {
      FILE *stream = getFilePointer(*(S_GPR + 3));
      char format[256] = {'\0'};
      getString(&format[0], 256, *(S_GPR + 4) + dram);
      /* PCRE section */
      split_t *_test;
      split_t *_temp;
      int _count = 5; /* arguments start from register 4 */
      int ret = 0;

      _test = pcre_split("%[\\+#-]?(\\d+|\\*)?\\.?\\d*([hlLzjt]|[hl]{2})?([csuxXfFeEpgGdionz])",format);
      _temp = _test;
      if(_test == NULL)
	printf("Error: Nothing to print\n");
      else {
	char _str[256] = {'\0'};
	do {
	  if(_test->string != NULL) {
	    /*printf("%s", _test->string);*/
	    strcat(_str, _test->string);
	  }

	  if(_test->match != NULL) {
	    strcat(_str, _test->match);
	    switch(*(char *)((size_t)_test->match + strlen(_test->match) - 1)) {
	    case 's':
	    case 'S':
	      {
		char str[256] = {'\0'};
		ret += fscanf(stream, _str, str);
		endianSwapLittle2Big((char *)(*(S_GPR + _count++) + dram), 1, strlen(str), str);
	      }
	    break;
	    case 'f':
	    case 'F':
	      {
		switch(*(char *)((size_t)_test->match + strlen(_test->match) - 2)) {
		case 'l':
		  {
		    double d;
		    ret += fscanf(stream, _str, &d);
		    int *dP = (int *)&d;
		    *(unsigned *)(*(S_GPR + _count) + dram) = *dP;
		    *(unsigned *)(*(S_GPR + _count++) + dram) = *(dP+1);
		  }
		  break;
		default:
		  {
		    float d;
		    ret += fscanf(stream, _str, &d);
		    int *dP = (int *)&d;
		    *(unsigned *)(*(S_GPR + _count++) + dram) = *dP;
		  }
		  break;
		}
	      }
	    break;
	    default:
	      {
		ret += fscanf(stream, _str, (unsigned *)(*(S_GPR + _count++) + dram));
	      }
	      break;
	    }
	    _str[0] = '\0';
	  }
	  if(_count > 10) {
	    printf("*** Limit of Insizzle printf ***\n");
	    break;
	  }
	} while((_test = _test->next) != NULL);
      }
			  
      /* TODO: fix this */
      /*pcre_split_free(_temp);*/

      *(S_GPR + 3) = (unsigned)ret;
    }
    break;
  case FGETC:
    /* r3 = fgetc(r3, r4) */
    {
      FILE *stream = getFilePointer(*(S_GPR + 3));

      int ret = fgetc(stream);

      *(S_GPR + 3) = (unsigned)ret;
    }
    break;
  case UNGETC:
    /* r3 = ungetc(r3) */
    {
      int c = (int)*(S_GPR + 3);
      FILE *stream = getFilePointer(*(S_GPR + 4));

      int ret = ungetc(c, stream);

      *(S_GPR + 3) = (unsigned)ret;
    }
    break;
  case ATOI:
    /* r3 = atoi(r3) */
    {
      char str[256] = {'\0'};
      getString(&str[0], 256, *(S_GPR + 3) + dram);

      int ret = atoi(str);

      /* endian swap */
      *(S_GPR + 3) = ret;/*((ret >> 24) & 0xff) | (((ret >> 16) & 0xff) << 8) | (((ret >> 8) & 0xff) << 16) | ((ret & 0xff) << 24);*/
    }
    break;
  case SRAND:
    /* srand(r3) */
    {
      unsigned seed = *(S_GPR + 3);

      srand(seed);
    }
    break;
  case RAND:
    /* r3 = rand() */
    {
      int ret = rand();

      /* endian swap */
      *(S_GPR + 3) = ret;/*((ret >> 24) & 0xff) | (((ret >> 16) & 0xff) << 8) | (((ret >> 8) & 0xff) << 16) | ((ret & 0xff) << 24);*/
    }
    break;
  case SPRINTF:
    /* r3 = sprintf(r3, r4, ...) */
    {
      char *s = (char *)(*(S_GPR + 3) + dram);
      char format[256] = {'\0'};
      getString(&format[0], 256, *(S_GPR + 4) + dram);

      /* make a char array the size of n */
      char *sLittle = calloc(1024, 1);

      /* PCRE section */
      split_t *_test;
      split_t *_temp;
      int _count = 5; /* arguments start from register 5 */

      _test = pcre_split("%[\\+#-]?(\\d+|\\*)?\\.?\\d*([hlLzjt]|[hl]{2})?([csuxXfFeEpgGdionz])",format);
      _temp = _test;
      if(_test == NULL)
	printf("Error: Nothing to print\n");
      else {
	do {
	  if(_test->string != NULL)
	    strcat(sLittle, _test->string);
			      
	  if(_test->match != NULL) {
	    switch(*(char *)((size_t)_test->match + strlen(_test->match) - 1)) {
	    case 's':
	    case 'S':
	      {
		char str[256] = {'\0'};
		getString(&str[0], 256, *(S_GPR + _count++) + dram);
		char _s[256];
		sprintf(_s, _test->match, str);
		strcat(sLittle, _s);
	      }
	    break;
	    case 'f':
	    case 'F':
	      {
		/* first register needs to be odd :S */
		if(!(_count % 2)) { _count++; } /* skip odd register */
		double d;
		int *dP = (int *)&d;
		*(dP+1) = *(S_GPR + _count++);
		*dP = *(S_GPR + _count++);

		char _s[256];
		sprintf(_s, _test->match, d);
		strcat(sLittle, _s);
	      }
	    break;
	    default:
	      {
		char _s[256];
		sprintf(_s, _test->match, *(S_GPR + _count++));
		strcat(sLittle, _s);
	      }
	      break;
	    }
	  }
	  if(_count > 10) {
	    printf("*** Limit of Insizzle printf ***\n");
	    break;
	  }
	} while((_test = _test->next) != NULL);
      }
			  
      /* TODO: fix this */
      /*pcre_split_free(_temp);*/

      /* need to copy sLittle into s */
      endianSwapLittle2Big(s, 1, strlen(sLittle), sLittle);
    }
    break;
  case SSCANF:
    /* r3 = sscanf(r3, r4, ...) */
    /* NOTE: limited to one match per string */
    {
      char s[256] = {'\0'};
      getString(&s[0], 256, *(S_GPR + 3) + dram);
      char format[256] = {'\0'};
      getString(&format[0], 256, *(S_GPR + 4) + dram);
      /* PCRE section */
      split_t *_test;
      split_t *_temp;
      int _count = 5; /* arguments start from register 4 */

      _test = pcre_split("%[\\+#-]?(\\d+|\\*)?\\.?\\d*([hlLzjt]|[hl]{2})?([csuxXfFeEpgGdionz])",format);
      _temp = _test;
      if(_test == NULL)
	printf("Error: Nothing to print\n");
      else {
	char _str[256] = {'\0'};
	do {
	  if(_test->string != NULL) {
	    /*printf("%s", _test->string);*/
	    strcat(_str, _test->string);
	  }

	  if(_test->match != NULL) {
	    strcat(_str, _test->match);
	    switch(*(char *)((size_t)_test->match + strlen(_test->match) - 1)) {
	    case 's':
	    case 'S':
	      {
		char str[256] = {'\0'};
		sscanf(s, _str, str);
		endianSwapLittle2Big((char *)(*(S_GPR + _count++) + dram), 1, strlen(str), str);
	      }
	    break;
	    case 'f':
	    case 'F':
	      {
		switch(*(char *)((size_t)_test->match + strlen(_test->match) - 2)) {
		case 'l':
		  {
		    double d;
		    sscanf(s, _str, &d);
		    int *dP = (int *)&d;
		    *(unsigned *)(*(S_GPR + _count) + dram) = *dP;
		    *(unsigned *)(*(S_GPR + _count++) + dram) = *(dP+1);
		  }
		  break;
		default:
		  {
		    float d;
		    sscanf(s, _str, &d);
		    int *dP = (int *)&d;
		    *(unsigned *)(*(S_GPR + _count++) + dram) = *dP;
		  }
		  break;
		}
	      }
	    break;
	    default:
	      sscanf(s, _str, (unsigned *)(*(S_GPR + _count++) + dram));
	      break;
	    }
	    _str[0] = '\0';
	  }
	  if(_count > 10) {
	    printf("*** Limit of Insizzle printf ***\n");
	    break;
	  }
	} while((_test = _test->next) != NULL);
      }
			  
      /* TODO: fix this */
      /*pcre_split_free(_temp);*/
    }
    break;
  case FPUTC:
    /* r3 = fputc(r3, r4) */
    {
      int c = (int)*(S_GPR + 3);
      FILE *stream = getFilePointer(*(S_GPR + 4));

      int ret = fputc(c, stream);

      *(S_GPR + 3) = (unsigned)ret;
    }
    break;
  case OPEN:
    /* r3 = open(r3, r4) */
    {
      char filename[256] = {'\0'};
      getString(&filename[0], 256, *(S_GPR + 3) + dram);
      int flags = (int)*(S_GPR + 4);
      int permissions = (int)*(S_GPR + 5);

      int ret = open(filename, flags, permissions);

      *(S_GPR + 3) = (unsigned)ret;
    }
    break;
  case CLOSE:
    /* r3 = close(r3) */
    {
      size_t fd = (size_t)getFilePointer(*(S_GPR + 3));

      int ret = close(fd);

      *(S_GPR + 3) = (unsigned)ret;
    }
    break;
  case READ:
    /* r3 = read(r3, r4, r5) */
    {
      int fd = (int)*(S_GPR + 3);
      void *ptr = (void *)(*(S_GPR + 4) + dram);
      int numbytes = (int)*(S_GPR + 5);

      /* copy into local memory then perform endian switch into LE1 space */
      char *ptrLittle = (char *)calloc(numbytes, 1);
      int ret = read(fd, (void *)ptrLittle, numbytes);
      endianSwapLittle2Big(ptr, 1, numbytes, ptrLittle);

      *(S_GPR + 3) = (unsigned)ret;
    }
    break;
  case WRITE:
    /* r3 = write(r3, r4, r5) */
    {
      int fd = (int)*(S_GPR + 3);
      void *ptr = (void *)(*(S_GPR + 4) + dram);
      int numbytes = (int)*(S_GPR + 5);

      /* copy into local memory then perform endian switch into LE1 space */
      char *ptrLittle = (char *)calloc(numbytes, 1);
      endianSwapBig2Little(ptrLittle, 1, numbytes, ptr);
      int ret = write(fd, (void *)ptrLittle, numbytes);


      *(S_GPR + 3) = (unsigned)ret;
    }
    break;
  case LSEEK:
    /* r3 = lseek(r3, r4, r5) */
    {
      int fd = (int)*(S_GPR + 3);
      int position = (int)*(S_GPR + 4);
      int startpoint = (int)*(S_GPR + 5);

      int ret = lseek(fd, position, startpoint);

      *(S_GPR + 3) = (unsigned)ret;
    }
    break;
  case FSTAT:
    /* r3 = fstat(r3, r4) */
    {
      int fd = (int)*(S_GPR + 3);
      /*struct stat *buf = (struct stat*)(*(S_GPR + 4) + dram);*/
      unsigned *buf = (unsigned *)(*(S_GPR + 4) + dram);

      /* this will require an endian flip of some kind :S */
      struct stat bufLittle;

      /* the struct elements and offset used in VEX
	 st_dev   : 0x0
	 st_ino   : 0x4 
	 st_mode  : 0x8
	 st_nlink : 0xc
	 st_uid   : 0x10
	 st_gid   : 0x14
	 st_rdev  : 0x18
	 st_size  : 0x1c
      */
      int ret = fstat(fd, &bufLittle);
      *(buf + 0) = bufLittle.st_dev;
      *(buf + 1) = bufLittle.st_ino;
      *(buf + 2) = bufLittle.st_mode;
      *(buf + 3) = bufLittle.st_nlink;
      *(buf + 4) = bufLittle.st_uid;
      *(buf + 5) = bufLittle.st_gid;
      *(buf + 6) = bufLittle.st_rdev;
      *(buf + 7) = bufLittle.st_size;
      
      *(S_GPR + 3) = (unsigned)ret;
    }
    break;
  case STRTOL:
    /* r3 = strtol(r3, r4, r5) */
    {
      char str[256] = {'\0'};
      getString(&str[0], 256, *(S_GPR + 3) + dram);
      int base = (int)*(S_GPR + 5);

      char *endptr;
      long ret = strtol(str, &endptr, base);

      /* then set the value of endptr in the LE1 */
      if(endptr == NULL) {
	/* leave it? */
      }
      else {
	unsigned *end_addr = (unsigned *)(*(S_GPR + 4) + dram);
	*end_addr = (*(S_GPR + 3) + (&(*endptr) - &str[0]));
      }

      *(S_GPR + 3) = (unsigned)ret;
    }
    break;
  case STRTOD:
    /* r3 = strtod(r3, r4) */
    /* TODO: THIS IS BROKEN */
    {
      char str[256] = {'\0'};
      getString(&str[0], 256, *(S_GPR + 3) + dram);

      char *endptr;
      float ret = strtof(str, &endptr);

      /* then set the value of endptr in the LE1 */
      if(endptr == NULL) {
	/* leave it? */
      }
      else {
	unsigned *end_addr = (unsigned *)(*(S_GPR + 4) + dram);
	*end_addr = (*(S_GPR + 3) + (&(*endptr) - &str[0]));
      }

      int *retP = (int *)&ret;
      *(S_GPR + 3) = (unsigned)*retP;
    }
    break;
  default:
    printf("Unknown SYSCALL: %d\n", call);
    break;
  }

}

/* switching endian, dependent on size */
void endianSwapLittle2Big(char *out, unsigned size, unsigned nitems, char *in) {
  /* data is in x86 endian, need to switch to LE1 */
  unsigned long memI = 0;
  unsigned i = 0;
  while(i < (nitems * size)) {
    switch(size) {
    case 2:
      _STH_iss((out + (memI * size)), ((unsigned short *)(in) + memI));
      i += 2;
      break;
    case 4:
      _STW_iss((out + (memI * size)), ((unsigned *)(in) + memI));
      i += 4;
      break;
    default:
      _STB_iss((out + memI), ((unsigned char*)(in) + memI));
      i++;
      break;
    }
    memI++;
  }
  return;
}

/* swap endian from LE1 to x86 for writing to file */
void endianSwapBig2Little(char *out, unsigned size, unsigned nitems, char *in) {
  /* data is in LE1 endian, need to switch to x86 */
  unsigned memI = 0;
  unsigned i = 0;
  while(i < (nitems * size)) {
    switch(size) {
    case 2:
      _LDUH_iss(((unsigned short *)out + memI), ((unsigned short*)(in) + memI));
      i += 2;
      break;
    case 4:
      _LDW_iss(((unsigned int *)out + memI), ((unsigned *)(in) + memI));
      i += 4;
      break;
    default:
      _LDUB_iss(((unsigned char *)out + memI), ((unsigned char*)(in) + memI));
      i++;
      break;
    }
    memI++;
  }
  return;
}

/* get a string from memory
   requires function due to endian stuff */
void getString(char *out, unsigned length, unsigned long memAddr) {
  unsigned stringI = 0;
  char c[2] = {'\0','\0'};
  while(stringI < length) {
    _LDUB_iss(&c[0], (memAddr + stringI));
    if(!c[0]) {
      return;
    }
    strcat(out, c);
    stringI++;
  }
}
