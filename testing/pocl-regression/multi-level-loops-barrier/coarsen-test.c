#include <stdio.h>

#define WINDOW_SIZE 32
#define WORK_ITEMS 16
#define BUFFER_SIZE (WORK_ITEMS + WINDOW_SIZE)

unsigned get_global_id(unsigned index) {
  return 0;
}
 
void test_kernel(int* input, 
                                  int* result) {
    int __kernel_local_id[3];
      __kernel_local_id[0] = 0;
        int gid[16];
          int j[16];
            int i[16];
              while (__kernel_local_id[0] < 16) {
                    gid[__kernel_local_id[0]] = get_global_id(0) + __kernel_local_id[0];
                        i[__kernel_local_id[0]] = 0;
                        j[__kernel_local_id[0]] = 0;
                        __kernel_local_id[0]++;
                          }
                for (i[__kernel_local_id[0]] = 0; i[__kernel_local_id[0]] < 32; ++i[__kernel_local_id[0]]) {
                      __kernel_local_id[0] = 0;
                          while (__kernel_local_id[0] < 16) {
                                  result[gid[__kernel_local_id[0]]] = input[gid[__kernel_local_id[0]]];
                                     
                                        __kernel_local_id[0]++;
                                            }
                              for (j[__kernel_local_id[0]] = 0; j[__kernel_local_id[0]] < i[__kernel_local_id[0]]; ++j[__kernel_local_id[0]]) {

                                      __kernel_local_id[0] = 0;
                                            while (__kernel_local_id[0] < 16) {
                                                      result[gid[__kernel_local_id[0]]] = input[gid[__kernel_local_id[0]]] * input[gid[__kernel_local_id[0]] + j[__kernel_local_id[0]]];
                                                              //barrier( 2  /*CLK_GLOBAL_MEM_FENCE*/);
                                                              __kernel_local_id[0]++;
                                                                    }

                                                  __kernel_local_id[0] = 0;
                                                        while (__kernel_local_id[0] < 16) {
                                                                  __kernel_local_id[0]++;
                                                                        }
                                                              
                                                            }
                                  __kernel_local_id[0] = 0;
                                      while (__kernel_local_id[0] < 16) {
                                              __kernel_local_id[0]++;
                                                  }
                                        }

                __kernel_local_id[0] = 0;
                while (__kernel_local_id[0] < 16) {
                  __kernel_local_id[0]++;
                }
}
int main(void) {
  int A[BUFFER_SIZE];
  int R[WORK_ITEMS];
  int a = 2;

  for (int i = 0; i < BUFFER_SIZE; i++) {
    A[i] = i;
  }

  for (int i = 0; i < WORK_ITEMS; i++) {
    R[i] = i;
  }

  test_kernel(A, R);
  for (int i = 0; i < WORK_ITEMS; ++i) {
    printf("res %d = %d\n", i, R[i]);
  }

}

