#define IN_RANGE(x, min, max) ((x)>=(min) && (x)<=(max))
#define CLAMP_RANGE(x, min, max) x = (x<(min)) ? min : ((x>(max)) ? max : x )
#define MIN(a, b) ((a)<=(b) ? (a) : (b))

__kernel void dynproc_kernel (int iteration,
                              __global int* gpuWall,
                              __global int* gpuSrc,
                              __global int* gpuResults,
                              int cols,
                              int rows,
                              int startStep,
                              int border,
                              int HALO,
                              __local int* prev,
                              __local int* result)
{int bx;
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
_Bool isValid[25];
_Bool computed[25];
for (__esdg_idx = 0; __esdg_idx < 25; ++__esdg_idx) {

	//int BLOCK_SIZE =

BLOCK_SIZE =  25;
	//int bx =

bx =  get_group_id(0);
	//int tx =

tx[__esdg_idx] =  __esdg_idx;

	

	// Each block finally computes result for a small block
	// after N iterations.
	// it is the non-overlapping small blocks that cover
	// all the input data

	// calculate the small block size.
	//int small_block_cols =

small_block_cols =  BLOCK_SIZE - (iteration*HALO*2);

	// calculate the boundary for the block according to
	// the boundary of its small block
	//int blkX =

blkX =  (small_block_cols*bx) - border;
	//int blkXmax =

blkXmax =  blkX+BLOCK_SIZE-1;

	// calculate the global thread coordination
	//int xidx =

xidx[__esdg_idx] =  blkX+tx[__esdg_idx];

	// effective range within this block that falls within
	// the valid range of the input data
	// used to rule out computation outside the boundary.
	//int validXmin =

validXmin =  (blkX < 0) ? -blkX : 0;
	//int validXmax =

validXmax =  (blkXmax > cols-1) ? BLOCK_SIZE-1-(blkXmax-cols+1) : BLOCK_SIZE-1;
	
	//int W =

W[__esdg_idx] =  tx[__esdg_idx]-1;
	//int E =

E[__esdg_idx] =  tx[__esdg_idx]+1;

	W[__esdg_idx] = (W[__esdg_idx] < validXmin) ? validXmin : W[__esdg_idx];
	E[__esdg_idx] = (E[__esdg_idx] > validXmax) ? validXmax : E[__esdg_idx];

	//bool isValid = 

isValid[__esdg_idx] =  ( ( tx[__esdg_idx] ) >= ( validXmin ) && ( tx[__esdg_idx] ) <= ( validXmax ) ) /*IN_RANGE*/ /*(tx, validXmin, validXmax)*/;

	if( ( ( xidx[__esdg_idx] ) >= ( 0 ) && ( xidx[__esdg_idx] ) <= ( cols - 1 ) )  /*IN_RANGE*/ /*(xidx, 0, cols-1)*/)
	{

		prev[tx[__esdg_idx]] = gpuSrc[xidx[__esdg_idx]];
	}


	
	//barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
} __esdg_idx = 0;

for (__esdg_idx = 0; __esdg_idx < 25; ++__esdg_idx) {


	

} __esdg_idx = 0;
	for (int i = 0; i < iteration; i++)
	{
	
for (__esdg_idx = 0; __esdg_idx < 25; ++__esdg_idx) {
	computed[__esdg_idx] = false;
		
		if(  ( ( tx[__esdg_idx] ) >= ( i + 1 ) && ( tx[__esdg_idx] ) <= ( BLOCK_SIZE - i - 2 ) ) /*IN_RANGE*/ /*(tx, i+1, BLOCK_SIZE-i-2)*/ && isValid[__esdg_idx] )
		{
			computed[__esdg_idx] = true;
			int left = prev[W[__esdg_idx]];
			int up = prev[tx[__esdg_idx]];
			int right = prev[E[__esdg_idx]];
			int shortest =  ( ( left ) <= ( up ) ? ( left ) : ( up ) ) /*MIN*/ /*(left, up)*/;
			shortest =  ( ( shortest ) <= ( right ) ? ( shortest ) : ( right ) ) /*MIN*/ /*(shortest, right)*/;
			
			int index = cols*(startStep+i)+xidx[__esdg_idx];
			result[tx[__esdg_idx]] = shortest + gpuWall[index];
			
		}

		//barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
} __esdg_idx = 0;

for (__esdg_idx = 0; __esdg_idx < 25; ++__esdg_idx) {

                printf("iteration = %d\n", iteration);

	
} __esdg_idx = 0;
	if(i==iteration-1)
		{
for (__esdg_idx = 0; __esdg_idx < 25; ++__esdg_idx) {

			// we are on the last iteration, and thus don't need to 
			// compute for the next step.
		
} __esdg_idx = 0;
	break;
for (__esdg_idx = 0; __esdg_idx < 25; ++__esdg_idx) {

		
} __esdg_idx = 0;
}
for (__esdg_idx = 0; __esdg_idx < 25; ++__esdg_idx) {


		if(computed[__esdg_idx])
		{
			//Assign the computation range
			prev[tx[__esdg_idx]] = result[tx[__esdg_idx]];
		}
		//barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
} __esdg_idx = 0;

for (__esdg_idx = 0; __esdg_idx < 25; ++__esdg_idx) {

	
} __esdg_idx = 0;
}

for (__esdg_idx = 0; __esdg_idx < 25; ++__esdg_idx) {

	// update the global memory
	// after the last iteration, only threads coordinated within the
	// small block perform the calculation and switch on "computed"
	if (computed[__esdg_idx])
	{
		gpuResults[xidx[__esdg_idx]] = result[tx[__esdg_idx]];
	}

__ESDG_END: ;
} __esdg_idx = 0;
}

