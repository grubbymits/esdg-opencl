#include <iostream>
#include <stdlib.h>
#include <time.h>

#define M_SEED 9
#define IN_RANGE(x, min, max)	((x)>=(min) && (x)<=(max))
#define CLAMP_RANGE(x, min, max) x = (x<(min)) ? min : ((x>(max)) ? max : x )
#define MIN(a, b) ((a)<=(b) ? (a) : (b))

void dynproc_kernel (int iteration,
                     int* gpuWall,
                     int* gpuSrc,
                     int* gpuResults,
                     int cols,
                     int rows,
                     int startStep,
                     int border,
                     int HALO,
                     int* prev,
                     int* result) {
  int bx;
  int small_block_cols;
  int blkX;
  int blkXmax;
  int validXmin;
  int validXmax;
  unsigned __esdg_idx = 0;
  int BLOCK_SIZE;
  int tx[25];
  int xidx[25];
  int W[25];
  int E[25];
  bool isValid[25];
  bool computed[25];

  std::cout << "gpuSrc = ";
  for (unsigned i = 0; i < 25; ++i) {
    std::cout << gpuSrc[i] << " ";
  }
  std::cout << std::endl;

  for (__esdg_idx = 0; __esdg_idx < 25; ++__esdg_idx) {
    BLOCK_SIZE =  25;

    bx =  0; //get_group_id(0);
    tx[__esdg_idx] =  __esdg_idx;
    small_block_cols =  BLOCK_SIZE - (iteration*HALO*2);
    blkX =  (small_block_cols*bx) - border;
    blkXmax =  blkX+BLOCK_SIZE-1;
    xidx[__esdg_idx] =  blkX+tx[__esdg_idx];
    validXmin =  (blkX < 0) ? -blkX : 0;
    validXmax =  (blkXmax > cols-1) ? BLOCK_SIZE-1-(blkXmax-cols+1) : BLOCK_SIZE-1;
    W[__esdg_idx] =  tx[__esdg_idx]-1;
    E[__esdg_idx] =  tx[__esdg_idx]+1;
    W[__esdg_idx] = (W[__esdg_idx] < validXmin) ? validXmin : W[__esdg_idx];
    E[__esdg_idx] = (E[__esdg_idx] > validXmax) ? validXmax : E[__esdg_idx];

    isValid[__esdg_idx] =
      (tx[__esdg_idx] >= validXmin) && (tx[__esdg_idx] <= validXmax);

    if ((xidx[__esdg_idx] >= 0) && (xidx[__esdg_idx] <= (cols - 1))) {
      prev[tx[__esdg_idx]] = gpuSrc[xidx[__esdg_idx]];
    }
  } __esdg_idx = 0;

  for (int i = 0; i < iteration; i++) {
    std::cout << "\niteration = " << i << std::endl;
    for (__esdg_idx = 0; __esdg_idx < 25; ++__esdg_idx) {
      computed[__esdg_idx] = false;

      if ((tx[__esdg_idx] >= ( i + 1 )) &&
          (tx[__esdg_idx] <= ( BLOCK_SIZE - i - 2 ))) {

        computed[__esdg_idx] = true;
	int left = prev[W[__esdg_idx]];
        int up = prev[tx[__esdg_idx]];
	int right = prev[E[__esdg_idx]];
	int shortest =  (left <= up ) ? left : up;
	shortest =  (shortest <= right ) ? shortest : right;
	int index = cols*(startStep+i)+xidx[__esdg_idx];

	result[tx[__esdg_idx]] = shortest + gpuWall[index];
        /*
        std::cout << "gpuWall = " << gpuWall[index] << ", left = " << left
          << ", up = " << up << ", right = " << right << ", so shortest = "
          << shortest << std::endl;
        std::cout << "idx = " << __esdg_idx << ", result[" << tx[__esdg_idx]
          << "] = " << result[tx[__esdg_idx]] << std::endl;*/
      }
    } __esdg_idx = 0;

    std::cout << "RESULT SO FAR:" << std::endl;
    for (unsigned i = 0; i < 5; ++i)
      std::cout << result[i] << std::endl;

    if(i==iteration-1)
	break;

    for (__esdg_idx = 0; __esdg_idx < 25; ++__esdg_idx) {
      if(computed[__esdg_idx]) {
        //Assign the computation range
	prev[tx[__esdg_idx]] = result[tx[__esdg_idx]];
      }
    } __esdg_idx = 0;
  }

  for (__esdg_idx = 0; __esdg_idx < 25; ++__esdg_idx) {
    // update the global memory
    // after the last iteration, only threads coordinated within the
    // small block perform the calculation and switch on "computed"
    if (computed[__esdg_idx]) {
      gpuResults[xidx[__esdg_idx]] = result[tx[__esdg_idx]];
    }

__ESDG_END: ;
  } __esdg_idx = 0;
}

int   rows, cols;
int   Ne = rows * cols;
int*  data;
int** wall;
//int*  result;

int* omp_data;
int** omp_wall;
int* omp_result;

void init() {
  data = new int[rows * cols];
  wall = new int*[rows];
  omp_data = new int[rows * cols];
  omp_wall = new int*[rows];

  for (int n = 0; n < rows; n++) {
    // wall[n] is set to be the nth row of the data array.
    wall[n] = data + cols * n;
    omp_wall[n] = omp_data + cols * n;
  }
  //result = new int[cols];
  omp_result = new int[cols];

  int seed = M_SEED;
  srand(seed);

  std::cout << "Wall data:" << std::endl;
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      wall[i][j] = rand() % 10;
      omp_wall[i][j] = wall[i][j];
      std::cout << wall[i][j] << " ";
    }
    std::cout << std::endl;
  }
}

void run_omp(void) {
  int *src, *dst, *temp;
  int min;

  dst = omp_result;
  src = new int[cols];

  for (int t = 0; t < rows-1; t++) {
    std::cout << "row = " << t << std::endl;
    temp = src;
    src = dst;
    dst = temp;
    #pragma omp parallel for private(min)
    for(int n = 0; n < cols; n++){
      min = src[n];
      if (n > 0)
        min = MIN(min, src[n-1]);
      if (n < cols-1)
        min = MIN(min, src[n+1]);
      dst[n] = omp_wall[t+1][n]+min;
      std::cout << "col = " << n << ", min = " << min
        << " and dst[" << n << "] = " 
        << "wall[" << t + 1 << "][" << n << "] + " << min
        << " = " << dst[n] << std::endl;
    }
  }
  std::cout << std::endl;
}

// run with 5 5 5
int main(void) {
  unsigned iteration = 4;
  cols = 5;
  rows = 5;
  unsigned borderCols = 5;
  unsigned startStep = 0;
  unsigned HALO = 1;

  init();
  run_omp();

  int *src = data;
  int gpuRes[5] = {0}; // cols
  int prev[25] = {0};
  int result[25] = {0};

  dynproc_kernel(iteration,
                 data + cols,
                 src,
                 gpuRes,
                 cols,
                 rows,
                 startStep,
                 borderCols,
                 HALO,
                 prev,
                 result);

  for (unsigned i = 0; i < cols; ++i) {
    std::cout << "result = " << result[i]
      << " and omp_result = " << omp_result[i] << std::endl;
  }

  delete[] data;
  delete[] wall;
  //delete[] result;
  delete[] omp_data;
  delete[] omp_wall;
  delete[] omp_result;
  return 0;
}

