	.file	"LICM_prog.profile.bc"
	.text
	.globl	main
	.align	16, 0x90
	.type	main,@function
main:                                   # @main
# BB#0:                                 # %entry
	subq	$24, %rsp
	xorl	%edi, %edi
	xorl	%esi, %esi
	movl	$EdgeProfCounters, %edx
	movl	$9, %ecx
	callq	llvm_start_edge_profiling
	incl	EdgeProfCounters(%rip)
	movl	$0, 12(%rsp)
	movl	$0, 8(%rsp)
	movl	$0, 4(%rsp)
	movl	$0, 12(%rsp)
	incl	EdgeProfCounters+4(%rip)
	jmp	.LBB0_5
	.align	16, 0x90
.LBB0_1:                                # %bb
                                        #   in Loop: Header=BB0_5 Depth=1
	incl	EdgeProfCounters+24(%rip)
	cmpb	$0, 12(%rsp)
	je	.LBB0_3
# BB#2:                                 # %bb.bb2_crit_edge
                                        #   in Loop: Header=BB0_5 Depth=1
	incl	EdgeProfCounters+12(%rip)
	jmp	.LBB0_4
.LBB0_3:                                # %bb1
                                        #   in Loop: Header=BB0_5 Depth=1
	incl	EdgeProfCounters+8(%rip)
	movl	12(%rsp), %eax
	movl	%eax, 8(%rsp)
	incl	EdgeProfCounters+16(%rip)
.LBB0_4:                                # %bb2
                                        #   in Loop: Header=BB0_5 Depth=1
	movl	8(%rsp), %eax
	leal	-4(,%rax,4), %eax
	movl	%eax, 4(%rsp)
	incl	12(%rsp)
	incl	EdgeProfCounters+20(%rip)
.LBB0_5:                                # %bb3
                                        # =>This Inner Loop Header: Depth=1
	cmpl	$100000000, 12(%rsp)    # imm = 0x5F5E100
	jl	.LBB0_1
# BB#6:                                 # %bb4
	incl	EdgeProfCounters+28(%rip)
	movl	4(%rsp), %esi
	movl	$.L.str, %edi
	xorb	%al, %al
	callq	printf
	movl	$0, 16(%rsp)
	movl	$0, 20(%rsp)
	incl	EdgeProfCounters+32(%rip)
	movl	20(%rsp), %eax
	addq	$24, %rsp
	ret
.Ltmp0:
	.size	main, .Ltmp0-main

	.type	.L.str,@object          # @.str
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str:
	.asciz	 "x = %d"
	.size	.L.str, 7

	.type	EdgeProfCounters,@object # @EdgeProfCounters
	.local	EdgeProfCounters        # @EdgeProfCounters
	.comm	EdgeProfCounters,36,16

	.section	".note.GNU-stack","",@progbits
