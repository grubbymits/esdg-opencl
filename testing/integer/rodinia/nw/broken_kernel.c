#define BLOCK_SIZE 16

#define SCORE(i, j) input_itemsets_l[j + i * (BLOCK_SIZE+1)]
#define REF(i, j)   reference_l[j + i * BLOCK_SIZE]

int get_group_id(int);
int get_global_id(int);

int maximum( int a,
		 int b,
		 int c){

	int k;
	if( a <= b )
		k = b;
	else 
	k = a;

	if( k <=c )
	return(c);
	else
	return(k);
}

void 
nw_kernel1(int  * reference_d, 
		   int  * input_itemsets_d, 
		   int  * output_itemsets_d, 
		   int  * input_itemsets_l,
		   int  * reference_l,
           int cols,
           int penalty,
           int blk,
           int block_width,
           int worksize,
           int offset_r,
           int offset_c
    )
{int tx[16];
int index[16];
int index_w[16];
int index_n[16];
int t_index_x[16];
int t_index_y[16];
   int __kernel_local_id[3];
__kernel_local_id[0] = 0;
while (__kernel_local_id[0] < 16) {
 

	// Block index
    int bx = get_group_id(0);	
	//int bx = get_global_id(0)/BLOCK_SIZE;
   
    /*// Thread index*/
     tx[__kernel_local_id[0]] = __kernel_local_id[0];
    
    // Base elements
    int base = offset_r * cols + offset_c;
    
    int b_index_x = bx;
	int b_index_y = blk - 1 - bx;
	
	
	 index[__kernel_local_id[0]]   =   base + cols *  16 /*BLOCK_SIZE*/ * b_index_y +  16 /*BLOCK_SIZE*/ * b_index_x + tx[__kernel_local_id[0]] + ( cols + 1 );
	 index_n[__kernel_local_id[0]]   = base + cols *  16 /*BLOCK_SIZE*/ * b_index_y +  16 /*BLOCK_SIZE*/ * b_index_x + tx[__kernel_local_id[0]] + ( 1 );
	 index_w[__kernel_local_id[0]]   = base + cols *  16 /*BLOCK_SIZE*/ * b_index_y +  16 /*BLOCK_SIZE*/ * b_index_x + ( cols );
	int index_nw =  base + cols *  16 /*BLOCK_SIZE*/ * b_index_y +  16 /*BLOCK_SIZE*/ * b_index_x;
   
    
	if (tx[__kernel_local_id[0]] == 0){
		 input_itemsets_l [ 0 + tx[__kernel_local_id[0]] * ( 16 + 1 ) ] /*SCORE*/ /*(tx, 0)*/ = input_itemsets_d[index_nw + tx[__kernel_local_id[0]]];
	}

	//barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
	__kernel_local_id[0]++;
	}
	__kernel_local_id[0] = 0;
	while (__kernel_local_id[0] < 16) {
	

	for ( int ty = 0 ; ty <  16 /*BLOCK_SIZE*/ ; ty++)
		 reference_l [ tx[__kernel_local_id[0]] + ty * 16 ] /*REF*/ /*(ty, tx)*/ =  reference_d[index[__kernel_local_id[0]] + cols * ty];

	//barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
	__kernel_local_id[0]++;
	}
	__kernel_local_id[0] = 0;
	while (__kernel_local_id[0] < 16) {
	

	 input_itemsets_l [ 0 + ( tx[__kernel_local_id[0]] + 1 ) * ( 16 + 1 ) ] /*SCORE*/ /*((tx + 1), 0)*/ = input_itemsets_d[index_w[__kernel_local_id[0]] + cols * tx[__kernel_local_id[0]]];

	//barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
	__kernel_local_id[0]++;
	}
	__kernel_local_id[0] = 0;
	while (__kernel_local_id[0] < 16) {
	

	 input_itemsets_l [ ( tx[__kernel_local_id[0]] + 1 ) + 0 * ( 16 + 1 ) ] /*SCORE*/ /*(0, (tx + 1))*/ = input_itemsets_d[index_n[__kernel_local_id[0]]];
  
	//barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
	__kernel_local_id[0]++;
	}
	__kernel_local_id[0] = 0;
	while (__kernel_local_id[0] < 16) {
	
	
	
	
	__kernel_local_id[0]++;
	}
	for( int m = 0 ; m <  16 /*BLOCK_SIZE*/ ; m++){
	__kernel_local_id[0] = 0;
while (__kernel_local_id[0] < 16) {
	
	  if ( tx[__kernel_local_id[0]] <= m ){
	  
		   t_index_x[__kernel_local_id[0]][__kernel_local_id[0]] =  tx[__kernel_local_id[0]] + 1;
		   t_index_y[__kernel_local_id[0]][__kernel_local_id[0]] =  m - tx[__kernel_local_id[0]] + 1;
			
		   input_itemsets_l [ t_index_x[__kernel_local_id[0]] + t_index_y[__kernel_local_id[0]] * ( 16 + 1 ) ] /*SCORE*/ /*(t_index_y, t_index_x)*/ = maximum(  input_itemsets_l [ ( t_index_x[__kernel_local_id[0]] - 1 ) + ( t_index_y[__kernel_local_id[0]] - 1 ) * ( 16 + 1 ) ] /*SCORE*/ /*((t_index_y-1), (t_index_x-1))*/ +  reference_l [ ( t_index_x[__kernel_local_id[0]] - 1 ) + ( t_index_y[__kernel_local_id[0]] - 1 ) * 16 ] /*REF*/ /*((t_index_y-1), (t_index_x-1))*/,
		                                          input_itemsets_l [ ( t_index_x[__kernel_local_id[0]] - 1 ) + ( t_index_y[__kernel_local_id[0]] ) * ( 16 + 1 ) ] /*SCORE*/ /*((t_index_y),   (t_index_x-1))*/ - (penalty), 
												  input_itemsets_l [ ( t_index_x[__kernel_local_id[0]] ) + ( t_index_y[__kernel_local_id[0]] - 1 ) * ( 16 + 1 ) ] /*SCORE*/ /*((t_index_y-1), (t_index_x))*/   - (penalty));
	  }
	  //barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
	  __kernel_local_id[0]++;
	  }
	  __kernel_local_id[0] = 0;
	  while (__kernel_local_id[0] < 16) {
	  
    
    __kernel_local_id[0]++;
    }
    }
__kernel_local_id[0] = 0;
while (__kernel_local_id[0] < 16) {
    
     //barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
     __kernel_local_id[0]++;
     }
     __kernel_local_id[0] = 0;
     while (__kernel_local_id[0] < 16) {
     
    
	
	__kernel_local_id[0]++;
	}
	for( int m =  16 /*BLOCK_SIZE*/ - 2 ; m >=0 ; m--){
 __kernel_local_id[0] = 0;
while (__kernel_local_id[0] < 16) {
   
	  if ( tx[__kernel_local_id[0]] <= m){
 
		   t_index_x =  tx[__kernel_local_id[0]] +  16 /*BLOCK_SIZE*/ - m ;
		   t_index_y =   16 /*BLOCK_SIZE*/ - tx[__kernel_local_id[0]];

          input_itemsets_l [ t_index_x[__kernel_local_id[0]] + t_index_y[__kernel_local_id[0]] * ( 16 + 1 ) ] /*SCORE*/ /*(t_index_y, t_index_x)*/ = maximum(   input_itemsets_l [ ( t_index_x[__kernel_local_id[0]] - 1 ) + ( t_index_y[__kernel_local_id[0]] - 1 ) * ( 16 + 1 ) ] /*SCORE*/ /*((t_index_y-1), (t_index_x-1))*/ +  reference_l [ ( t_index_x[__kernel_local_id[0]] - 1 ) + ( t_index_y[__kernel_local_id[0]] - 1 ) * 16 ] /*REF*/ /*((t_index_y-1), (t_index_x-1))*/,
		                                          input_itemsets_l [ ( t_index_x[__kernel_local_id[0]] - 1 ) + ( t_index_y[__kernel_local_id[0]] ) * ( 16 + 1 ) ] /*SCORE*/ /*((t_index_y),   (t_index_x-1))*/ - (penalty), 
		 										  input_itemsets_l [ ( t_index_x[__kernel_local_id[0]] ) + ( t_index_y[__kernel_local_id[0]] - 1 ) * ( 16 + 1 ) ] /*SCORE*/ /*((t_index_y-1), (t_index_x))*/   - (penalty));
	   
	  }

	  //barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
	  __kernel_local_id[0]++;
	  }
	  __kernel_local_id[0] = 0;
	  while (__kernel_local_id[0] < 16) {
	  
	
	__kernel_local_id[0]++;
	}
	}
__kernel_local_id[0] = 0;
while (__kernel_local_id[0] < 16) {
	

   for ( int ty = 0 ; ty <  16 /*BLOCK_SIZE*/ ; ty++)
     input_itemsets_d[index[__kernel_local_id[0]] + cols * ty] =  input_itemsets_l [ ( tx[__kernel_local_id[0]] + 1 ) + ( ty + 1 ) * ( 16 + 1 ) ] /*SCORE*/ /*((ty+1), (tx+1))*/;
    
    return;
   

__kernel_local_id[0]++;
}
}