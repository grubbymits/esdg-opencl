/*
 * A self-written benchmark to show constant propagation 
 * after superblock formation
 */

#include <stdio.h>
#include <stdlib.h>

int main() {
  /* seed the random number generator */
  srand(time(NULL));

  int MAXITER = 100000000;
  int THRESHOLD = 100;
  int val = 40;

  int idx, num;
  int sum = 0;
  for (idx = 0; idx < MAXITER; ++idx) {
    if (rand() % THRESHOLD > THRESHOLD - 10) {
      // less taken path
      num = num + val;
    } else {
      // more likely to take this path (90 % of the time)
      num = 10;
    }
    if (rand() % THRESHOLD > THRESHOLD - 10) {
      // less taken path
      num = num + val;
    } else {
      // more likely to take this path (90 % of the time)
      num = 10;
    }
    if (rand() % THRESHOLD > THRESHOLD - 10) {
      // less taken path
      num = num + val;
    } else {
      // more likely to take this path (90 % of the time)
      num = 10;
    }

    if (rand() % THRESHOLD > THRESHOLD - 10) {
      // less taken path
      num = num + val;
    } else {
      // more likely to take this path (90 % of the time)
      num = 10;
    }

    if (rand() % THRESHOLD > THRESHOLD - 10) {
      // less taken path
      num = num + val;
    } else {
      // more likely to take this path (90 % of the time)
      num = 10;
    }

    if (rand() % THRESHOLD > THRESHOLD - 10) {
      // less taken path
      num = num + val;
    } else {
      // more likely to take this path (90 % of the time)
      num = 10;
    }

    if (rand() % THRESHOLD > THRESHOLD - 10) {
      // less taken path
      num = num + val;
    } else {
      // more likely to take this path (90 % of the time)
      num = 10;
    }

    // able to do constant propagation after cloning BB
    val = num + num;

    // a check to see if modified code behaves correctly
    sum += idx;
  }
  
  printf("Sum = %d\n", sum);
  return 0;
}
