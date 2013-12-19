/* ============================================================
//--cambine: kernel funtion of Breadth-First-Search
//--author:	created by Jianbin Fang
//--date:	06/12/2010
============================================================ */
#pragma OPENCL EXTENSION cl_khr_byte_addressable_store: enable
//Structure to hold a node information
typedef struct{
	int starting;
	int no_of_edges;
} Node;
//--7 parameters


//--5 parameters
__kernel void BFS_2(__global char* restrict g_graph_mask, 
		    __global char* restrict g_updating_graph_mask, 
		    __global char* restrict g_graph_visited, 
		    __global char* restrict g_over,
		    const  int no_of_nodes) {  unsigned __esdg_idx = 0;
for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {

  int tid = __builtin_le1_read_group_id_0() * 256 + __esdg_idx;//get_global_id(0);
  if( tid<no_of_nodes && g_updating_graph_mask[tid]){

    g_graph_mask[tid]=true;
    g_graph_visited[tid]=true;
    *g_over=true;
    g_updating_graph_mask[tid]=false;
  }

__ESDG_END: ;
} __esdg_idx = 0;
}



