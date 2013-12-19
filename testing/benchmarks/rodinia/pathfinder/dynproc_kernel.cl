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
{
	int BLOCK_SIZE = get_local_size(0);
	int bx = get_group_id(0);
	int tx = get_local_id(0);

	

	// Each block finally computes result for a small block
	// after N iterations.
	// it is the non-overlapping small blocks that cover
	// all the input data

	// calculate the small block size.
	int small_block_cols = BLOCK_SIZE - (iteration*HALO*2);

	// calculate the boundary for the block according to
	// the boundary of its small block
	int blkX = (small_block_cols*bx) - border;
	int blkXmax = blkX+BLOCK_SIZE-1;

	// calculate the global thread coordination
	int xidx = blkX+tx;

	// effective range within this block that falls within
	// the valid range of the input data
	// used to rule out computation outside the boundary.
	int validXmin = (blkX < 0) ? -blkX : 0;
	int validXmax = (blkXmax > cols-1) ? BLOCK_SIZE-1-(blkXmax-cols+1) : BLOCK_SIZE-1;
	
	int W = tx-1;
	int E = tx+1;

	W = (W < validXmin) ? validXmin : W;
	E = (E > validXmax) ? validXmax : E;

	bool isValid =  ( ( tx ) >= ( validXmin ) && ( tx ) <= ( validXmax ) ) /*IN_RANGE*/ /*(tx, validXmin, validXmax)*/;

	if( ( ( xidx ) >= ( 0 ) && ( xidx ) <= ( cols - 1 ) )  /*IN_RANGE*/ /*(xidx, 0, cols-1)*/)
	{

		prev[tx] = gpuSrc[xidx];
	}


	
	barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);

	bool computed;
	for (int i = 0; i < iteration; i++)
	{
		computed = false;
		
		if(  ( ( tx ) >= ( i + 1 ) && ( tx ) <= ( BLOCK_SIZE - i - 2 ) ) /*IN_RANGE*/ /*(tx, i+1, BLOCK_SIZE-i-2)*/ && isValid )
		{
			computed = true;
			int left = prev[W];
			int up = prev[tx];
			int right = prev[E];
			int shortest =  ( ( left ) <= ( up ) ? ( left ) : ( up ) ) /*MIN*/ /*(left, up)*/;
			shortest =  ( ( shortest ) <= ( right ) ? ( shortest ) : ( right ) ) /*MIN*/ /*(shortest, right)*/;
			
			int index = cols*(startStep+i)+xidx;
			result[tx] = shortest + gpuWall[index];
			
		}

		barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
                printf("iteration = %d\n", iteration);

		if(i==iteration-1)
		{
			// we are on the last iteration, and thus don't need to 
			// compute for the next step.
			break;
		}

		if(computed)
		{
			//Assign the computation range
			prev[tx] = result[tx];
		}
		barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
	}

	// update the global memory
	// after the last iteration, only threads coordinated within the
	// small block perform the calculation and switch on "computed"
	if (computed)
	{
		gpuResults[xidx] = result[tx];
	}
}

