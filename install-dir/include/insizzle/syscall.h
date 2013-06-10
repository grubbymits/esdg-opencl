#include "syscall_layout.h"

void endianSwapLittle2Big(char *, unsigned, unsigned, char *); /* from x86 to LE1 */
void endianSwapBig2Little(char *, unsigned, unsigned, char *); /* from LE1 to x86 */
void getString(char *, unsigned, unsigned long);
