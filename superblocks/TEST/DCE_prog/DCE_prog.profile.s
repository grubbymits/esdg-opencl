	.file	"DCE_prog.profile.bc"
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
	movl	$14, %ecx
	callq	llvm_start_edge_profiling
	incl	EdgeProfCounters(%rip)
	movl	$0, 8(%rsp)
	movl	$0, 4(%rsp)
	movl	$0, 12(%rsp)
	incl	EdgeProfCounters+4(%rip)
	jmp	.LBB0_8
	.align	16, 0x90
.LBB0_1:                                # %bb
                                        #   in Loop: Header=BB0_8 Depth=1
	incl	EdgeProfCounters+44(%rip)
	cmpl	$98999999, 12(%rsp)     # imm = 0x5E69EBF
	jg	.LBB0_3
# BB#2:                                 # %bb1
                                        #   in Loop: Header=BB0_8 Depth=1
	incl	EdgeProfCounters+8(%rip)
	movl	$1, 8(%rsp)
	incl	EdgeProfCounters+16(%rip)
	jmp	.LBB0_4
.LBB0_3:                                # %bb2
                                        #   in Loop: Header=BB0_8 Depth=1
	incl	EdgeProfCounters+12(%rip)
	movl	4(%rsp), %eax
	addl	%eax, 8(%rsp)
	incl	EdgeProfCounters+20(%rip)
.LBB0_4:                                # %bb3
                                        #   in Loop: Header=BB0_8 Depth=1
	movl	8(%rsp), %eax
	movl	%eax, 4(%rsp)
	cmpl	$98999999, 12(%rsp)     # imm = 0x5E69EBF
	jg	.LBB0_6
# BB#5:                                 # %bb4
                                        #   in Loop: Header=BB0_8 Depth=1
	incl	EdgeProfCounters+24(%rip)
	incl	4(%rsp)
	incl	EdgeProfCounters+32(%rip)
	jmp	.LBB0_7
.LBB0_6:                                # %bb5
                                        #   in Loop: Header=BB0_8 Depth=1
	incl	EdgeProfCounters+28(%rip)
	decl	4(%rsp)
	incl	EdgeProfCounters+36(%rip)
.LBB0_7:                                # %bb6
                                        #   in Loop: Header=BB0_8 Depth=1
	incl	12(%rsp)
	incl	EdgeProfCounters+40(%rip)
.LBB0_8:                                # %bb7
                                        # =>This Inner Loop Header: Depth=1
	cmpl	$100000000, 12(%rsp)    # imm = 0x5F5E100
	jl	.LBB0_1
# BB#9:                                 # %bb8
	incl	EdgeProfCounters+48(%rip)
	movl	8(%rsp), %esi
	movl	$.L.str, %edi
	xorb	%al, %al
	callq	printf
	movl	4(%rsp), %esi
	movl	$.L.str1, %edi
	xorb	%al, %al
	callq	printf
	movl	$.L.str2, %edi
	movl	12(%rsp), %esi
	xorb	%al, %al
	callq	printf
	movl	$0, 16(%rsp)
	movl	$0, 20(%rsp)
	incl	EdgeProfCounters+52(%rip)
	movl	20(%rsp), %eax
	addq	$24, %rsp
	ret
.Ltmp0:
	.size	main, .Ltmp0-main

	.type	.L.str,@object          # @.str
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str:
	.asciz	 "a = %d\n"
	.size	.L.str, 8

	.type	.L.str1,@object         # @.str1
.L.str1:
	.asciz	 "b = %d\n"
	.size	.L.str1, 8

	.type	.L.str2,@object         # @.str2
.L.str2:
	.asciz	 "i = %d\n"
	.size	.L.str2, 8

	.type	EdgeProfCounters,@object # @EdgeProfCounters
	.local	EdgeProfCounters        # @EdgeProfCounters
	.comm	EdgeProfCounters,56,16

	.section	".note.GNU-stack","",@progbits
