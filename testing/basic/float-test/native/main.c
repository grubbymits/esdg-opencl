#include "softfloat.h"

#ifdef NATIVE
#include <stdio.h>
#endif

// perform ((-30006 * -26069) + 1.0)
// = 782226415
// = 2E9FD3EF

//int a = 0;
//int b = 0;
int c = 0;
int d = 0;
int e = 0;
int f = 0;

int main(void) {
  int a = 25261;
  int b = 1682;

  c = a * b;
  d = int32_to_float32(c);
  e = float32_add(d, 0x3f800000);
  f = float32_to_int32(e);

#ifdef NATIVE
  printf("softfloat conversion = %x\n", d);
  printf("softfloat add res = %x\n", e);
  printf("final softfloat result = %x\n", f);

  float native_float = a * b + 1.0;
  unsigned int native = native_float;

  printf("Native result = %x\n", native);
#endif
  return f;
}
