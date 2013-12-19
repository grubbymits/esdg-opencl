#define BLOCK_SIZE 16

#define SCORE(i, j) input_itemsets_l[j + i * (BLOCK_SIZE+1)]
#define REF(i, j)   reference_l[j + i * BLOCK_SIZE]

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



__kernel void 
nw_kernel2(__global int  * reference_d, 
		   __global int  * input_itemsets_d, 
		   //__global int  * output_itemsets_d, 
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
{ int bx;
int base;
int b_index_x;
int b_index_y;
int index[16];
int index_n[16];
int index_w;
int index_nw;
  unsigned __esdg_idx = 0;
int tx[16];
for (__esdg_idx = 0; __esdg_idx < 16; ++__esdg_idx) {
 

	//int bx =

bx =  get_group_id(0);	
	//int bx = get_global_id(0)/BLOCK_SIZE;
   
    // Thread index
    //int tx =

tx[__esdg_idx] =  __esdg_idx;
    
    // Base elements
    //int base =

base =  offset_r * cols + offset_c;
    
    //int b_index_x =

b_index_x =  bx + block_width - blk  ;
	//int b_index_y =

b_index_y =  block_width - bx -1;
	
	
	//int index   =  

index[__esdg_idx] =  base + cols *  16 /*BLOCK_SIZE*/ * b_index_y +  16 /*BLOCK_SIZE*/ * b_index_x + tx[__esdg_idx] + ( cols + 1 );
	//int index_n   =

index_n[__esdg_idx] =  base + cols *  16 /*BLOCK_SIZE*/ * b_index_y +  16 /*BLOCK_SIZE*/ * b_index_x + tx[__esdg_idx] + ( 1 );
	//int index_w   =

index_w =  base + cols *  16 /*BLOCK_SIZE*/ * b_index_y +  16 /*BLOCK_SIZE*/ * b_index_x + ( cols );
	//int index_nw = 

index_nw =  base + cols *  16 /*BLOCK_SIZE*/ * b_index_y +  16 /*BLOCK_SIZE*/ * b_index_x;
    
	if (tx[__esdg_idx] == 0)
		 input_itemsets_l [ 0 + tx[__esdg_idx] * ( 16 + 1 ) ] /*SCORE*/ /*(tx, 0)*/ = input_itemsets_d[index_nw];

	for ( int ty = 0 ; ty <  16 /*BLOCK_SIZE*/ ; ty++)
		 reference_l [ tx[__esdg_idx] + ty * 16 ] /*REF*/ /*(ty, tx)*/ =  reference_d[index[__esdg_idx] + cols * ty];

	//barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
} __esdg_idx = 0;

for (__esdg_idx = 0; __esdg_idx < 16; ++__esdg_idx) {


	 input_itemsets_l [ 0 + ( tx[__esdg_idx] + 1 ) * ( 16 + 1 ) ] /*SCORE*/ /*((tx + 1), 0)*/ = input_itemsets_d[index_w + cols * tx[__esdg_idx]];

	//barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
} __esdg_idx = 0;

for (__esdg_idx = 0; __esdg_idx < 16; ++__esdg_idx) {


	 input_itemsets_l [ ( tx[__esdg_idx] + 1 ) + 0 * ( 16 + 1 ) ] /*SCORE*/ /*(0, (tx + 1))*/ = input_itemsets_d[index_n[__esdg_idx]];
  
	//barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
} __esdg_idx = 0;

for (__esdg_idx = 0; __esdg_idx < 16; ++__esdg_idx) {

  

} __esdg_idx = 0;
	for( int m = 0 ; m <  16 /*BLOCK_SIZE*/ ; m++){
	
for (__esdg_idx = 0; __esdg_idx < 16; ++__esdg_idx) {

	  if ( tx[__esdg_idx] <= m ){
	  
		  int t_index_x =  tx[__esdg_idx] + 1;
		  int t_index_y =  m - tx[__esdg_idx] + 1;

          input_itemsets_l [ t_index_x + t_index_y * ( 16 + 1 ) ] /*SCORE*/ /*(t_index_y, t_index_x)*/ = maximum(   input_itemsets_l [ ( t_index_x - 1 ) + ( t_index_y - 1 ) * ( 16 + 1 ) ] /*SCORE*/ /*((t_index_y-1), (t_index_x-1))*/ +  reference_l [ ( t_index_x - 1 ) + ( t_index_y - 1 ) * 16 ] /*REF*/ /*((t_index_y-1), (t_index_x-1))*/,
		                                          input_itemsets_l [ ( t_index_x - 1 ) + ( t_index_y ) * ( 16 + 1 ) ] /*SCORE*/ /*((t_index_y),   (t_index_x-1))*/ - (penalty), 
		 										  input_itemsets_l [ ( t_index_x ) + ( t_index_y - 1 ) * ( 16 + 1 ) ] /*SCORE*/ /*((t_index_y-1), (t_index_x))*/   - (penalty));
	  }
	  //barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
} __esdg_idx = 0;

for (__esdg_idx = 0; __esdg_idx < 16; ++__esdg_idx) {

    
} __esdg_idx = 0;
}

for (__esdg_idx = 0; __esdg_idx < 16; ++__esdg_idx) {


} __esdg_idx = 0;
	for( int m =  16 /*BLOCK_SIZE*/ - 2 ; m >=0 ; m--){
 
for (__esdg_idx = 0; __esdg_idx < 16; ++__esdg_idx) {
  
	  if ( tx[__esdg_idx] <= m){
 
		  int t_index_x =  tx[__esdg_idx] +  16 /*BLOCK_SIZE*/ - m ;
		  int t_index_y =   16 /*BLOCK_SIZE*/ - tx[__esdg_idx];

           input_itemsets_l [ t_index_x + t_index_y * ( 16 + 1 ) ] /*SCORE*/ /*(t_index_y, t_index_x)*/ = maximum(  input_itemsets_l [ ( t_index_x - 1 ) + ( t_index_y - 1 ) * ( 16 + 1 ) ] /*SCORE*/ /*((t_index_y-1), (t_index_x-1))*/ +  reference_l [ ( t_index_x - 1 ) + ( t_index_y - 1 ) * 16 ] /*REF*/ /*((t_index_y-1), (t_index_x-1))*/,
		                                          input_itemsets_l [ ( t_index_x - 1 ) + ( t_index_y ) * ( 16 + 1 ) ] /*SCORE*/ /*((t_index_y),   (t_index_x-1))*/ - (penalty), 
		 										  input_itemsets_l [ ( t_index_x ) + ( t_index_y - 1 ) * ( 16 + 1 ) ] /*SCORE*/ /*((t_index_y-1), (t_index_x))*/   - (penalty));
	   
	  }

	  //barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
} __esdg_idx = 0;

for (__esdg_idx = 0; __esdg_idx < 16; ++__esdg_idx) {

	
} __esdg_idx = 0;
}

for (__esdg_idx = 0; __esdg_idx < 16; ++__esdg_idx) {

	for ( int ty = 0 ; ty <  16 /*BLOCK_SIZE*/ ; ty++)
		input_itemsets_d[index[__esdg_idx] + ty * cols] =  input_itemsets_l [ ( tx[__esdg_idx] + 1 ) + ( ty + 1 ) * ( 16 + 1 ) ] /*SCORE*/ /*((ty+1), (tx+1))*/;
	
     
} __esdg_idx = 0;
 return;
for (__esdg_idx = 0; __esdg_idx < 16; ++__esdg_idx) {

  

__ESDG_END: ;
} __esdg_idx = 0;
}

