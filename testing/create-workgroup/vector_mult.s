	.file	"vector_mult.bc"
	.text
	.globl	main
	.align	2
	.type	main,@function
main:                                   # @main
# BB#0:                                 # %entry
	sub.0  r0.1, r0.1, 0x8
	;;
	mov.0  r0.3, 0x40
	;;
	stw.0  r0.1[0x4], l0.0
	;;                              # 4-byte Folded Spill
	mov.0  r0.4, 0x50
	;;
	mov.0  r0.5, 0x60
	;;
	call.0  l0.0, vector_mult
	;;
	mov.0  r0.2, r0.0
	;;
	ldw.0  l0.0, r0.1[0x4]
	;;                              # 4-byte Folded Reload
	mov.0  r0.3, r0.0
	;;
	call.0  l0.0, exit
	;;
	return.0  r0.1, r0.1, 0x0, l0.0
	;;
$tmp0:
	.size	main, ($tmp0)-main

	.globl	vector_mult
	.align	2
	.type	vector_mult,@function
vector_mult:                            # @vector_mult
# BB#0:                                 # %entry.barrier
	sub.0  r0.1, r0.1, 0x10
	;;
	stw.0  r0.1[0x8], r0.57
	;;                              # 4-byte Folded Spill
	mov.0  r0.57, r0.5
	;;
	stw.0  r0.1[0x4], r0.58
	;;                              # 4-byte Folded Spill
	mov.0  r0.58, r0.3
	;;
	stw.0  r0.1[0x0], r0.59
	;;                              # 4-byte Folded Spill
	mov.0  r0.59, r0.4
	;;
	stw.0  r0.1[0xc], l0.0
	;;                              # 4-byte Folded Spill
	call.0  l0.0, pocl.barrier
	;;
	ldw.0 r0.3, r0.0[(local_size+0)]
	;;
	cpuid.0  r0.2
	;;
	mullu.0  r0.4, r0.3, r0.2
	;;
	mulhs.0  r0.3, r0.3, r0.2
	;;
	add.0  r0.3, r0.4, r0.3
	;;
	add.0  r0.5, r0.3, r0.2
	;;
	sh4add.0  r0.4, r0.5, r0.58
	;;
	sh4add.0  r0.3, r0.5, r0.59
	;;
	ldw.0  r0.7, r0.4[0xc]
	;;
	sh4add.0  r0.5, r0.5, r0.57
	;;
	ldw.0  r0.8, r0.3[0xc]
	;;
	ldw.0  r0.9, r0.4[0x8]
	;;
	mulhs.0  r0.6, r0.7, r0.8
	;;
	ldw.0  r0.10, r0.3[0x8]
	;;
	mullu.0  r0.7, r0.7, r0.8
	;;
	ldw.0  r0.12, r0.3[0x4]
	;;
	mulhs.0  r0.8, r0.9, r0.10
	;;
	ldw.0  r0.3, r0.3[0x0]
	;;
	mullu.0  r0.9, r0.9, r0.10
	;;
	ldw.0  r0.10, r0.4[0x4]
	;;
	add.0  r0.6, r0.7, r0.6
	;;
	ldw.0  r0.4, r0.4[0x0]
	;;
	mulhs.0  r0.11, r0.10, r0.12
	;;
	stw.0  r0.5[0xc], r0.6
	;;
	mullu.0  r0.10, r0.10, r0.12
	;;
	add.0  r0.8, r0.9, r0.8
	;;
	mullu.0  r0.6, r0.4, r0.3
	;;
	add.0  r0.10, r0.10, r0.11
	;;
	mulhs.0  r0.3, r0.4, r0.3
	;;
	stw.0  r0.5[0x8], r0.8
	;;
	add.0  r0.3, r0.6, r0.3
	;;
	stw.0  r0.5[0x4], r0.10
	;;
	stw.0  r0.5[0x0], r0.3
	;;
	ldw.0 r0.3, r0.0[(local_size+0)]
	;;
	mullu.0  r0.4, r0.3, r0.2
	;;
	mulhs.0  r0.3, r0.3, r0.2
	;;
	add.0  r0.3, r0.4, r0.3
	;;
	add.0  r0.5, r0.3, r0.2
	;;
	sh4add.0  r0.4, r0.5, r0.58
	;;
	sh4add.0  r0.3, r0.5, r0.59
	;;
	ldw.0  r0.7, r0.4[0xc]
	;;
	sh4add.0  r0.5, r0.5, r0.57
	;;
	ldw.0  r0.8, r0.3[0xc]
	;;
	ldw.0  r0.9, r0.4[0x8]
	;;
	mulhs.0  r0.6, r0.7, r0.8
	;;
	ldw.0  r0.10, r0.3[0x8]
	;;
	mullu.0  r0.7, r0.7, r0.8
	;;
	ldw.0  r0.12, r0.3[0x4]
	;;
	mulhs.0  r0.8, r0.9, r0.10
	;;
	ldw.0  r0.3, r0.3[0x0]
	;;
	mullu.0  r0.9, r0.9, r0.10
	;;
	ldw.0  r0.10, r0.4[0x4]
	;;
	add.0  r0.6, r0.7, r0.6
	;;
	ldw.0  r0.4, r0.4[0x0]
	;;
	mulhs.0  r0.11, r0.10, r0.12
	;;
	stw.0  r0.5[0xc], r0.6
	;;
	mullu.0  r0.10, r0.10, r0.12
	;;
	add.0  r0.8, r0.9, r0.8
	;;
	mullu.0  r0.6, r0.4, r0.3
	;;
	add.0  r0.10, r0.10, r0.11
	;;
	mulhs.0  r0.3, r0.4, r0.3
	;;
	stw.0  r0.5[0x8], r0.8
	;;
	add.0  r0.3, r0.6, r0.3
	;;
	stw.0  r0.5[0x4], r0.10
	;;
	stw.0  r0.5[0x0], r0.3
	;;
	ldw.0 r0.3, r0.0[(local_size+0)]
	;;
	mullu.0  r0.4, r0.3, r0.2
	;;
	mulhs.0  r0.3, r0.3, r0.2
	;;
	add.0  r0.3, r0.4, r0.3
	;;
	add.0  r0.4, r0.3, r0.2
	;;
	sh4add.0  r0.3, r0.4, r0.58
	;;
	sh4add.0  r0.2, r0.4, r0.59
	;;
	ldw.0  r0.6, r0.3[0xc]
	;;
	sh4add.0  r0.4, r0.4, r0.57
	;;
	ldw.0  r0.7, r0.2[0xc]
	;;
	ldw.0  r0.8, r0.3[0x8]
	;;
	mulhs.0  r0.5, r0.6, r0.7
	;;
	ldw.0  r0.9, r0.2[0x8]
	;;
	mullu.0  r0.6, r0.6, r0.7
	;;
	ldw.0  r0.11, r0.2[0x4]
	;;
	mulhs.0  r0.7, r0.8, r0.9
	;;
	ldw.0  r0.2, r0.2[0x0]
	;;
	mullu.0  r0.8, r0.8, r0.9
	;;
	ldw.0  r0.9, r0.3[0x4]
	;;
	add.0  r0.5, r0.6, r0.5
	;;
	ldw.0  r0.3, r0.3[0x0]
	;;
	mulhs.0  r0.10, r0.9, r0.11
	;;
	stw.0  r0.4[0xc], r0.5
	;;
	mullu.0  r0.9, r0.9, r0.11
	;;
	add.0  r0.7, r0.8, r0.7
	;;
	mullu.0  r0.5, r0.3, r0.2
	;;
	add.0  r0.9, r0.9, r0.10
	;;
	mulhs.0  r0.2, r0.3, r0.2
	;;
	stw.0  r0.4[0x8], r0.7
	;;
	add.0  r0.2, r0.5, r0.2
	;;
	stw.0  r0.4[0x4], r0.9
	;;
	stw.0  r0.4[0x0], r0.2
	;;
	call.0  l0.0, pocl.barrier
	;;
	ldw.0  l0.0, r0.1[0xc]
	;;                              # 4-byte Folded Reload
	ldw.0  r0.59, r0.1[0x0]
	;;                              # 4-byte Folded Reload
	ldw.0  r0.58, r0.1[0x4]
	;;                              # 4-byte Folded Reload
	ldw.0  r0.57, r0.1[0x8]
	;;                              # 4-byte Folded Reload
	return.0  r0.1, r0.1, 0x10, l0.0
	;;
$tmp1:
	.size	vector_mult, ($tmp1)-vector_mult

	.type	work_dim,@object        # @work_dim
	.section	.sdata,"aw",@progbits
	.globl	work_dim
	.align	2
work_dim:
	.4byte	2                       # 0x2
	.size	work_dim, 4

	.type	global_size,@object     # @global_size
	.data
	.globl	global_size
	.align	2
global_size:
	.4byte	1024                    # 0x400
	.4byte	1024                    # 0x400
	.4byte	0                       # 0x0
	.size	global_size, 12

	.type	local_size,@object      # @local_size
	.globl	local_size
	.align	2
local_size:
	.4byte	256                     # 0x100
	.4byte	256                     # 0x100
	.4byte	0                       # 0x0
	.size	local_size, 12

	.type	num_groups,@object      # @num_groups
	.globl	num_groups
	.align	2
num_groups:
	.4byte	4                       # 0x4
	.4byte	4                       # 0x4
	.4byte	0                       # 0x0
	.size	num_groups, 12

	.type	global_offset,@object   # @global_offset
	.bss
	.globl	global_offset
	.align	2
global_offset:
	.space	12
	.size	global_offset, 12


