	.file	"BFS_1.bc"
	.text
	.globl	main
	.align	2
	.type	main,@function
main:                                   # @main
# BB#0:                                 # %entry
	sub.0  r0.1, r0.1, 0x8
	;;
	stw.0  r0.1[0x4], l0.0
	stw.0  r0.1[0x0], r0.0
	;;
	mov.0  r0.3, 0x2c
	mov.0  r0.4, 0x802c
	;;
	mov.0  r0.5, 0x2002c
	mov.0  r0.6, 0x2102c
	;;
	mov.0  r0.7, 0x2202c
	mov.0  r0.8, 0x2302c
	;;
	mov.0  r0.9, 0x1000
	call.0  l0.0, BFS_1
	;;
	mov.0  r0.3, r0.0
	ldw.0  l0.0, r0.1[0x4]
	;;
	call.0  l0.0, exit
	;;
$tmp0:
	.size	main, ($tmp0)-main

	.globl	BFS_1
	.align	2
	.type	BFS_1,@function
BFS_1:                                  # @BFS_1
# BB#0:                                 # %entry
	cpuid.0  r0.2
	;;
	shru.0  r0.2, r0.2, 0x8
	ldw.0 r0.10, r0.0[(local_size+0)]
	;;
	mullu.0  r0.11, r0.2, r0.10
	mulhs.0  r0.2, r0.2, r0.10
	;;
	add.0  r0.2, r0.11, r0.2
	mov.0  r0.10, r0.0
	;;
	mov.0  r0.11, 0x1
	mov.0  r0.12, r0.0
	;;
$BB1_1:                                 # %while.body6
                                        # =>This Loop Header: Depth=1
                                        #     Child Loop BB1_5 Depth 2
	add.0  r0.13, r0.2, r0.12
	;;
	cmpge.0  b0.0, r0.13, r0.9      # if (tid < no_of_nodes)
	;;
	br.0  b0.0, ($BB1_8)
	;;
	goto.0  ($BB1_2)
	;;
$BB1_2:                                 # %land.lhs.true
                                        #   in Loop: Header=BB1_1 Depth=1
	add.0  r0.15, r0.5, r0.13
	;;
	ldbu.0  r0.14, r0.15[0x0]
	;;
	cmpeq.0  b0.0, r0.14, r0.0      # if (g_graph_mask[tid])
	;;
	br.0  b0.0, ($BB1_8)
	;;
	goto.0  ($BB1_3)
	;;
$BB1_3:                                 # %if.then
                                        #   in Loop: Header=BB1_1 Depth=1
	sh3add.0  r0.14, r0.13, r0.3
	stb.0  r0.15[0x0], r0.10        # g_graph_mask[tid] = r0.10
	;;
	ldw.0  r0.16, r0.14[0x4]
	;;
	cmplt.0  b0.0, r0.16, 0x1
	;;
	br.0  b0.0, ($BB1_8)
	;;
	goto.0  ($BB1_4)
	;;
$BB1_4:                                 # %for.body.lr.ph
                                        #   in Loop: Header=BB1_1 Depth=1
	ldw.0  r0.17, r0.14[0x0]
	;;
	mov.0  r0.15, r0.17
	;;
$BB1_5:                                 # %for.body
                                        #   Parent Loop BB1_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	sh2add.0  r0.18, r0.15, r0.4
	;;
	ldw.0  r0.18, r0.18[0x0]
	;;
	add.0  r0.19, r0.7, r0.18
	;;
	ldbu.0  r0.19, r0.19[0x0]     # r0.19 = g_graph_visited[id]
	;;
	cmpne.0  b0.0, r0.19, r0.0    # if (!g_graph_visited[id])
	;;
	br.0  b0.0, ($BB1_7)
	;;
	goto.0  ($BB1_6)
	;;
$BB1_6:                                 # %if.then21
                                        #   in Loop: Header=BB1_5 Depth=2
	sh2add.0  r0.16, r0.13, r0.8    # r0.8 = g_cost
	;;
	ldw.0  r0.16, r0.16[0x0]        # r0.16 = g_cost[tid]
	;;
	add.0  r0.16, r0.16, 0x1
	sh2add.0  r0.17, r0.18, r0.8
	;;
	stw.0  r0.17[0x0], r0.16        # r0.17 = g_cost[id]
	add.0  r0.16, r0.6, r0.18
	;;
	stb.0  r0.16[0x0], r0.11        # g_updating_graph_mask[id] = true;
	ldw.0  r0.17, r0.14[0x0]        # r0.14 = g_graph_nodes[tid]
	;;
	ldw.0  r0.16, r0.14[0x4]
	;;
$BB1_7:                                 # %for.inc
                                        #   in Loop: Header=BB1_5 Depth=2
	add.0  r0.15, r0.15, 0x1
	add.0  r0.18, r0.17, r0.16
	;;
	cmplt.0  b0.0, r0.15, r0.18     # if (i < g_graph_nodes[tid].no_of_edges
                                        # + g_graph_nodes
	;;
	br.0  b0.0, ($BB1_5)
	;;
	goto.0  ($BB1_8)
	;;
$BB1_8:                                 # %if.end26
                                        #   in Loop: Header=BB1_1 Depth=1
	add.0  r0.12, r0.12, 0x1
	;;
	cmpne.0  b0.0, r0.12, 0x1000
	;;
	br.0  b0.0, ($BB1_1)
	;;
	goto.0  ($BB1_9)
	;;
$BB1_9:                                 # %while.end31
	return.0  r0.1, r0.1, 0x0, l0.0
	;;
$tmp1:
	.size	BFS_1, ($tmp1)-BFS_1

	.globl	BFS_2
	.align	2
	.type	BFS_2,@function
BFS_2:                                  # @BFS_2
# BB#0:                                 # %entry
	cpuid.0  r0.2
	;;
	shru.0  r0.2, r0.2, 0x8
	ldw.0 r0.8, r0.0[(local_size+0)]
	;;
	mullu.0  r0.9, r0.2, r0.8
	mulhs.0  r0.2, r0.2, r0.8
	;;
	add.0  r0.2, r0.9, r0.2
	mov.0  r0.8, r0.0
	;;
	add.0  r0.4, r0.4, r0.2
	add.0  r0.3, r0.3, r0.2
	;;
	add.0  r0.5, r0.5, r0.2
	mov.0  r0.9, 0x1
	;;
	mov.0  r0.10, r0.0
	;;
$BB2_1:                                 # %while.body6
                                        # =>This Inner Loop Header: Depth=1
	add.0  r0.11, r0.2, r0.10
	;;
	cmpge.0  b0.0, r0.11, r0.7
	;;
	br.0  b0.0, ($BB2_4)
	;;
	goto.0  ($BB2_2)
	;;
$BB2_2:                                 # %land.lhs.true
                                        #   in Loop: Header=BB2_1 Depth=1
	add.0  r0.11, r0.4, r0.10
	;;
	ldbu.0  r0.12, r0.11[0x0]
	;;
	cmpeq.0  b0.0, r0.12, r0.0
	;;
	br.0  b0.0, ($BB2_4)
	;;
	goto.0  ($BB2_3)
	;;
$BB2_3:                                 # %if.then
                                        #   in Loop: Header=BB2_1 Depth=1
	add.0  r0.12, r0.3, r0.10
	;;
	stb.0  r0.12[0x0], r0.9
	add.0  r0.12, r0.5, r0.10
	;;
	stb.0  r0.12[0x0], r0.9
	stb.0  r0.6[0x0], r0.9
	;;
	stb.0  r0.11[0x0], r0.8
	;;
$BB2_4:                                 # %if.end
                                        #   in Loop: Header=BB2_1 Depth=1
	add.0  r0.10, r0.10, 0x1
	;;
	cmpne.0  b0.0, r0.10, 0x1000
	;;
	br.0  b0.0, ($BB2_1)
	;;
	goto.0  ($BB2_5)
	;;
$BB2_5:                                 # %while.end16
	return.0  r0.1, r0.1, 0x0, l0.0
	;;
$tmp2:
	.size	BFS_2, ($tmp2)-BFS_2


