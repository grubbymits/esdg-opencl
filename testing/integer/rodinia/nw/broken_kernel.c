#define BLOCK_SIZE 16

#define SCORE(i, j) input_itemsets_l[j + i * (BLOCK_SIZE+1)]
#define REF(i, j)   reference_l[j + i * BLOCK_SIZE]

int maximum( int a,
		 int b,
  int __kernel_local_id[3];
		 __kernel_local_id[0] = 0;
		 while (__kernel_local_id[0] < 16) {
		 

	int k;
	if( a <= b )
		k = b;
	else 
	k = a;

	if( k <=c )
	return(c);
	else
	return(k);

__kernel_local_id[0]++;
}
}

__kernel void 
nw_kernel1(__global int  * reference_d, 
		   __global int  * input_itemsets_d, 
		   __global int  * output_itemsets_d, 
		   __local	int  * input_itemsets_l,
		   __local	int  * reference_l,
           int cols,
           int penalty,
           int blk,
           int block_width,
           int worksize,
           int offset_r,
           int offset_c
    )
{   int __kernel_local_id[3];
__kernel_local_id[0] = 0;
while (__kernel_local_id[0] < 16) {
 

	// Block index
    int bx = get_group_id(0);	
	//int bx = get_global_id(0)/BLOCK_SIZE;
   
    /*// Thread index*/
    int tx = __kernel_local_id[0];
    
    // Base elements
    int base = offset_r * cols + offset_c;
    
    int b_index_x = bx;
	int b_index_y = blk - 1 - bx;
	
	
	int index   =   base + cols *  16 /*BLOCK_SIZE*/ * b_index_y +  16 /*BLOCK_SIZE*/ * b_index_x + tx + ( cols + 1 );
	int index_n   = base + cols *  16 /*BLOCK_SIZE*/ * b_index_y +  16 /*BLOCK_SIZE*/ * b_index_x + tx + ( 1 );
	int index_w   = base + cols *  16 /*BLOCK_SIZE*/ * b_index_y +  16 /*BLOCK_SIZE*/ * b_index_x + ( cols );
	int index_nw =  base + cols *  16 /*BLOCK_SIZE*/ * b_index_y +  16 /*BLOCK_SIZE*/ * b_index_x;
   
    
	if (tx == 0){
		 input_itemsets_l [ 0 + tx * ( 16 + 1 ) ] /*SCORE*/ /*(tx, 0)*/ = input_itemsets_d[index_nw + tx];
	}

	//barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
	__kernel_local_id[0]++;
	}
	__kernel_local_id[0] = 0;
	while (__kernel_local_id[0] < 16) {
	

	for ( int ty = 0 ; ty <  16 /*BLOCK_SIZE*/ ; ty++)
		 reference_l [ tx + ty * 16 ] /*REF*/ /*(ty, tx)*/ =  reference_d[index + cols * ty];

	barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);

	 input_itemsets_l [ 0 + ( tx + 1 ) * ( 16 + 1 ) ] /*SCORE*/ /*((tx + 1), 0)*/ = input_itemsets_d[index_w + cols * tx];

	barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);

	 input_itemsets_l [ ( tx + 1 ) + 0 * ( 16 + 1 ) ] /*SCORE*/ /*(0, (tx + 1))*/ = input_itemsets_d[index_n];
  
	barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
	
	
	for( int m = 0 ; m <  16 /*BLOCK_SIZE*/ ; m++){
	
	  if ( tx <= m ){
	  
		  int t_index_x =  tx + 1;
		  int t_index_y =  m - tx + 1;
			
		   input_itemsets_l [ t_index_x + t_index_y * ( 16 + 1 ) ] /*SCORE*/ /*(t_index_y, t_index_x)*/ = maximum(  input_itemsets_l [ ( t_index_x - 1 ) + ( t_index_y - 1 ) * ( 16 + 1 ) ] /*SCORE*/ /*((t_index_y-1), (t_index_x-1))*/ +  reference_l [ ( t_index_x - 1 ) + ( t_index_y - 1 ) * 16 ] /*REF*/ /*((t_index_y-1), (t_index_x-1))*/,
		                                          input_itemsets_l [ ( t_index_x - 1 ) + ( t_index_y ) * ( 16 + 1 ) ] /*SCORE*/ /*((t_index_y),   (t_index_x-1))*/ - (penalty), 
												  input_itemsets_l [ ( t_index_x ) + ( t_index_y - 1 ) * ( 16 + 1 ) ] /*SCORE*/ /*((t_index_y-1), (t_index_x))*/   - (penalty));
	  }
	  barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
    }
    
     barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
    
	for( int m =  16 /*BLOCK_SIZE*/ - 2 ; m >=0 ; m--){
   
	  if ( tx <= m){
 
		  int t_index_x =  tx +  16 /*BLOCK_SIZE*/ - m ;
		  int t_index_y =   16 /*BLOCK_SIZE*/ - tx;

          input_itemsets_l [ t_index_x + t_index_y * ( 16 + 1 ) ] /*SCORE*/ /*(t_index_y, t_index_x)*/ = maximum(   input_itemsets_l [ ( t_index_x - 1 ) + ( t_index_y - 1 ) * ( 16 + 1 ) ] /*SCORE*/ /*((t_index_y-1), (t_index_x-1))*/ +  reference_l [ ( t_index_x - 1 ) + ( t_index_y - 1 ) * 16 ] /*REF*/ /*((t_index_y-1), (t_index_x-1))*/,
		                                          input_itemsets_l [ ( t_index_x - 1 ) + ( t_index_y ) * ( 16 + 1 ) ] /*SCORE*/ /*((t_index_y),   (t_index_x-1))*/ - (penalty), 
		 										  input_itemsets_l [ ( t_index_x ) + ( t_index_y - 1 ) * ( 16 + 1 ) ] /*SCORE*/ /*((t_index_y-1), (t_index_x))*/   - (penalty));
	   
	  }

	  barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
	}
	

   for ( int ty = 0 ; ty <  16 /*BLOCK_SIZE*/ ; ty++)
     input_itemsets_d[index + cols * ty] =  input_itemsets_l [ ( tx + 1 ) + ( ty + 1 ) * ( 16 + 1 ) ] /*SCORE*/ /*((ty+1), (tx+1))*/;
    
    return;
   

__kernel_local_id[0]++;
}
}

__kernel void 
nw_kernel2(__global int  * reference_d, 
		   __global int  * input_itemsets_d, 
		   __global int  * output_itemsets_d, 
		   __local	int  * input_itemsets_l,
		   __local	int  * reference_l,
           int cols,
           int penalty,
           int blk,
           int block_width,
           int worksize,
           int offset_r,
           int offset_c
    )
{   int __kernel_local_id[3];
__kernel_local_id[0] = 0;
while (__kernel_local_id[0] < 16) {
 

	int bx = get_group_id(0);	
	//int bx = get_global_id(0)/BLOCK_SIZE;
   
    /*// Thread index*/
    int tx = __kernel_local_id[0];
    
    // Base elements
    int base = offset_r * cols + offset_c;
    
    int b_index_x = bx + block_width - blk  ;
	int b_index_y = block_width - bx -1;
	
	
	int index   =   base + cols *  16 /*BLOCK_SIZE*/ * b_index_y +  16 /*BLOCK_SIZE*/ * b_index_x + tx + ( cols + 1 );
	int index_n   = base + cols *  16 /*BLOCK_SIZE*/ * b_index_y +  16 /*BLOCK_SIZE*/ * b_index_x + tx + ( 1 );
	int index_w   = base + cols *  16 /*BLOCK_SIZE*/ * b_index_y +  16 /*BLOCK_SIZE*/ * b_index_x + ( cols );
	int index_nw =  base + cols *  16 /*BLOCK_SIZE*/ * b_index_y +  16 /*BLOCK_SIZE*/ * b_index_x;
    
	if (tx == 0)
		 input_itemsets_l [ 0 + tx * ( 16 + 1 ) ] /*SCORE*/ /*(tx, 0)*/ = input_itemsets_d[index_nw];

	for ( int ty = 0 ; ty <  16 /*BLOCK_SIZE*/ ; ty++)
		 reference_l [ tx + ty * 16 ] /*REF*/ /*(ty, tx)*/ =  reference_d[index + cols * ty];

	barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);

	 input_itemsets_l [ 0 + ( tx + 1 ) * ( 16 + 1 ) ] /*SCORE*/ /*((tx + 1), 0)*/ = input_itemsets_d[index_w + cols * tx];

	barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);

	 input_itemsets_l [ ( tx + 1 ) + 0 * ( 16 + 1 ) ] /*SCORE*/ /*(0, (tx + 1))*/ = input_itemsets_d[index_n];
  
	barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
  
	for( int m = 0 ; m <  16 /*BLOCK_SIZE*/ ; m++){
	
	  if ( tx <= m ){
	  
		  int t_index_x =  tx + 1;
		  int t_index_y =  m - tx + 1;

          input_itemsets_l [ t_index_x + t_index_y * ( 16 + 1 ) ] /*SCORE*/ /*(t_index_y, t_index_x)*/ = maximum(   input_itemsets_l [ ( t_index_x - 1 ) + ( t_index_y - 1 ) * ( 16 + 1 ) ] /*SCORE*/ /*((t_index_y-1), (t_index_x-1))*/ +  reference_l [ ( t_index_x - 1 ) + ( t_index_y - 1 ) * 16 ] /*REF*/ /*((t_index_y-1), (t_index_x-1))*/,
		                                          input_itemsets_l [ ( t_index_x - 1 ) + ( t_index_y ) * ( 16 + 1 ) ] /*SCORE*/ /*((t_index_y),   (t_index_x-1))*/ - (penalty), 
		 										  input_itemsets_l [ ( t_index_x ) + ( t_index_y - 1 ) * ( 16 + 1 ) ] /*SCORE*/ /*((t_index_y-1), (t_index_x))*/   - (penalty));
	  }
	  barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
    }

	for( int m =  16 /*BLOCK_SIZE*/ - 2 ; m >=0 ; m--){
   
	  if ( tx <= m){
 
		  int t_index_x =  tx +  16 /*BLOCK_SIZE*/ - m ;
		  int t_index_y =   16 /*BLOCK_SIZE*/ - tx;

           input_itemsets_l [ t_index_x + t_index_y * ( 16 + 1 ) ] /*SCORE*/ /*(t_index_y, t_index_x)*/ = maximum(  input_itemsets_l [ ( t_index_x - 1 ) + ( t_index_y - 1 ) * ( 16 + 1 ) ] /*SCORE*/ /*((t_index_y-1), (t_index_x-1))*/ +  reference_l [ ( t_index_x - 1 ) + ( t_index_y - 1 ) * 16 ] /*REF*/ /*((t_index_y-1), (t_index_x-1))*/,
		                                          input_itemsets_l [ ( t_index_x - 1 ) + ( t_index_y ) * ( 16 + 1 ) ] /*SCORE*/ /*((t_index_y),   (t_index_x-1))*/ - (penalty), 
		 										  input_itemsets_l [ ( t_index_x ) + ( t_index_y - 1 ) * ( 16 + 1 ) ] /*SCORE*/ /*((t_index_y-1), (t_index_x))*/   - (penalty));
	   
	  }

	  barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
	}

	for ( int ty = 0 ; ty <  16 /*BLOCK_SIZE*/ ; ty++)
		input_itemsets_d[index + ty * cols] =  input_itemsets_l [ ( tx + 1 ) + ( ty + 1 ) * ( 16 + 1 ) ] /*SCORE*/ /*((ty+1), (tx+1))*/;
	
    
    return;
  

__kernel_local_id[0]++;
}
}
