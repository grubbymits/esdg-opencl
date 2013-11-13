	.file	"<stdin>"
	.text
	.globl	main
	.align	16, 0x90
	.type	main,@function
main:                                   # @main
# BB#0:                                 # %entry
	subq	$40, %rsp
	xorl	%edi, %edi
	xorb	%al, %al
	callq	time
	movl	%eax, %edi
	callq	srand
	movl	$100000000, 28(%rsp)    # imm = 0x5F5E100
	movl	$100, 24(%rsp)
	movl	$40, 20(%rsp)
	movl	$0, 8(%rsp)
	movl	$0, 16(%rsp)
	jmp	.LBB0_23
	.align	16, 0x90
.LBB0_1:                                # %bb
                                        #   in Loop: Header=BB0_23 Depth=1
	callq	rand
	movl	24(%rsp), %ecx
	cltd
	idivl	%ecx
	addl	$-10, %ecx
	cmpl	%ecx, %edx
	jle	.LBB0_3
# BB#2:                                 # %bb1
                                        #   in Loop: Header=BB0_23 Depth=1
	movl	20(%rsp), %eax
	addl	%eax, 12(%rsp)
	jmp	.LBB0_4
.LBB0_3:                                # %bb2
                                        #   in Loop: Header=BB0_23 Depth=1
	movl	$10, 12(%rsp)
.LBB0_4:                                # %bb3
                                        #   in Loop: Header=BB0_23 Depth=1
	callq	rand
	movl	24(%rsp), %ecx
	cltd
	idivl	%ecx
	addl	$-10, %ecx
	cmpl	%ecx, %edx
	jle	.LBB0_6
# BB#5:                                 # %bb4
                                        #   in Loop: Header=BB0_23 Depth=1
	movl	20(%rsp), %eax
	addl	%eax, 12(%rsp)
	jmp	.LBB0_7
.LBB0_6:                                # %bb5
                                        #   in Loop: Header=BB0_23 Depth=1
	movl	$10, 12(%rsp)
.LBB0_7:                                # %bb6
                                        #   in Loop: Header=BB0_23 Depth=1
	callq	rand
	movl	24(%rsp), %ecx
	cltd
	idivl	%ecx
	addl	$-10, %ecx
	cmpl	%ecx, %edx
	jle	.LBB0_9
# BB#8:                                 # %bb7
                                        #   in Loop: Header=BB0_23 Depth=1
	movl	20(%rsp), %eax
	addl	%eax, 12(%rsp)
	jmp	.LBB0_10
.LBB0_9:                                # %bb8
                                        #   in Loop: Header=BB0_23 Depth=1
	movl	$10, 12(%rsp)
.LBB0_10:                               # %bb9
                                        #   in Loop: Header=BB0_23 Depth=1
	callq	rand
	movl	24(%rsp), %ecx
	cltd
	idivl	%ecx
	addl	$-10, %ecx
	cmpl	%ecx, %edx
	jle	.LBB0_12
# BB#11:                                # %bb10
                                        #   in Loop: Header=BB0_23 Depth=1
	movl	20(%rsp), %eax
	addl	%eax, 12(%rsp)
	jmp	.LBB0_13
.LBB0_12:                               # %bb11
                                        #   in Loop: Header=BB0_23 Depth=1
	movl	$10, 12(%rsp)
.LBB0_13:                               # %bb12
                                        #   in Loop: Header=BB0_23 Depth=1
	callq	rand
	movl	24(%rsp), %ecx
	cltd
	idivl	%ecx
	addl	$-10, %ecx
	cmpl	%ecx, %edx
	jle	.LBB0_15
# BB#14:                                # %bb13
                                        #   in Loop: Header=BB0_23 Depth=1
	movl	20(%rsp), %eax
	addl	%eax, 12(%rsp)
	jmp	.LBB0_16
.LBB0_15:                               # %bb14
                                        #   in Loop: Header=BB0_23 Depth=1
	movl	$10, 12(%rsp)
.LBB0_16:                               # %bb15
                                        #   in Loop: Header=BB0_23 Depth=1
	callq	rand
	movl	24(%rsp), %ecx
	cltd
	idivl	%ecx
	addl	$-10, %ecx
	cmpl	%ecx, %edx
	jle	.LBB0_18
# BB#17:                                # %bb16
                                        #   in Loop: Header=BB0_23 Depth=1
	movl	20(%rsp), %eax
	addl	%eax, 12(%rsp)
	jmp	.LBB0_19
.LBB0_18:                               # %bb17
                                        #   in Loop: Header=BB0_23 Depth=1
	movl	$10, 12(%rsp)
.LBB0_19:                               # %bb18
                                        #   in Loop: Header=BB0_23 Depth=1
	callq	rand
	movl	24(%rsp), %ecx
	cltd
	idivl	%ecx
	addl	$-10, %ecx
	cmpl	%ecx, %edx
	jle	.LBB0_21
# BB#20:                                # %bb19
                                        #   in Loop: Header=BB0_23 Depth=1
	movl	20(%rsp), %eax
	addl	%eax, 12(%rsp)
	jmp	.LBB0_22
.LBB0_21:                               # %bb20
                                        #   in Loop: Header=BB0_23 Depth=1
	movl	$10, 12(%rsp)
.LBB0_22:                               # %bb21
                                        #   in Loop: Header=BB0_23 Depth=1
	movl	12(%rsp), %eax
	addl	%eax, %eax
	movl	%eax, 20(%rsp)
	movl	16(%rsp), %eax
	addl	%eax, 8(%rsp)
	incl	16(%rsp)
.LBB0_23:                               # %bb22
                                        # =>This Inner Loop Header: Depth=1
	movl	16(%rsp), %eax
	cmpl	28(%rsp), %eax
	jl	.LBB0_1
# BB#24:                                # %bb23
	movl	8(%rsp), %esi
	movl	$.L.str, %edi
	xorb	%al, %al
	callq	printf
	movl	$0, 32(%rsp)
	movl	$0, 36(%rsp)
	xorl	%eax, %eax
	addq	$40, %rsp
	ret
.Ltmp0:
	.size	main, .Ltmp0-main

	.type	.L.str,@object          # @.str
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str:
	.asciz	 "Sum = %d\n"
	.size	.L.str, 10


	.section	".note.GNU-stack","",@progbits
