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
	jmp	.LBB0_1
	.align	16, 0x90
.LBB0_12:                               # %bb1
                                        #   in Loop: Header=BB0_1 Depth=1
	movl	20(%rsp), %eax
	addl	%eax, 12(%rsp)
	callq	rand
	movl	24(%rsp), %ecx
	cltd
	idivl	%ecx
	addl	$-10, %ecx
	cmpl	%ecx, %edx
	jg	.LBB0_14
# BB#13:                                # %bb5.cloned
                                        #   in Loop: Header=BB0_1 Depth=1
	movl	$10, 12(%rsp)
	jmp	.LBB0_15
.LBB0_14:                               # %bb4
                                        #   in Loop: Header=BB0_1 Depth=1
	movl	20(%rsp), %eax
	addl	%eax, 12(%rsp)
.LBB0_15:                               # %bb6.cloned
                                        #   in Loop: Header=BB0_1 Depth=1
	callq	rand
	movl	24(%rsp), %ecx
	cltd
	idivl	%ecx
	addl	$-10, %ecx
	cmpl	%ecx, %edx
	jg	.LBB0_17
# BB#16:                                # %bb8.cloned
                                        #   in Loop: Header=BB0_1 Depth=1
	movl	$10, 12(%rsp)
	jmp	.LBB0_18
.LBB0_17:                               # %bb7
                                        #   in Loop: Header=BB0_1 Depth=1
	movl	20(%rsp), %eax
	addl	%eax, 12(%rsp)
.LBB0_18:                               # %bb9.cloned
                                        #   in Loop: Header=BB0_1 Depth=1
	callq	rand
	movl	24(%rsp), %ecx
	cltd
	idivl	%ecx
	addl	$-10, %ecx
	cmpl	%ecx, %edx
	jg	.LBB0_20
# BB#19:                                # %bb11.cloned
                                        #   in Loop: Header=BB0_1 Depth=1
	movl	$10, 12(%rsp)
	jmp	.LBB0_21
.LBB0_20:                               # %bb10
                                        #   in Loop: Header=BB0_1 Depth=1
	movl	20(%rsp), %eax
	addl	%eax, 12(%rsp)
.LBB0_21:                               # %bb12.cloned
                                        #   in Loop: Header=BB0_1 Depth=1
	callq	rand
	movl	24(%rsp), %ecx
	cltd
	idivl	%ecx
	addl	$-10, %ecx
	cmpl	%ecx, %edx
	jg	.LBB0_23
# BB#22:                                # %bb14.cloned
                                        #   in Loop: Header=BB0_1 Depth=1
	movl	$10, 12(%rsp)
	jmp	.LBB0_24
.LBB0_23:                               # %bb13
                                        #   in Loop: Header=BB0_1 Depth=1
	movl	20(%rsp), %eax
	addl	%eax, 12(%rsp)
.LBB0_24:                               # %bb15.cloned
                                        #   in Loop: Header=BB0_1 Depth=1
	callq	rand
	movl	24(%rsp), %ecx
	cltd
	idivl	%ecx
	addl	$-10, %ecx
	cmpl	%ecx, %edx
	jg	.LBB0_26
# BB#25:                                # %bb17.cloned
                                        #   in Loop: Header=BB0_1 Depth=1
	movl	$10, 12(%rsp)
	jmp	.LBB0_27
.LBB0_26:                               # %bb16
                                        #   in Loop: Header=BB0_1 Depth=1
	movl	20(%rsp), %eax
	addl	%eax, 12(%rsp)
.LBB0_27:                               # %bb18.cloned
                                        #   in Loop: Header=BB0_1 Depth=1
	callq	rand
	movl	24(%rsp), %ecx
	cltd
	idivl	%ecx
	addl	$-10, %ecx
	cmpl	%ecx, %edx
	jg	.LBB0_29
# BB#28:                                # %bb20.cloned
                                        #   in Loop: Header=BB0_1 Depth=1
	movl	$10, 12(%rsp)
	jmp	.LBB0_30
.LBB0_29:                               # %bb19
                                        #   in Loop: Header=BB0_1 Depth=1
	movl	20(%rsp), %eax
	addl	%eax, 12(%rsp)
.LBB0_30:                               # %bb21.cloned
                                        #   in Loop: Header=BB0_1 Depth=1
	movl	12(%rsp), %eax
	addl	%eax, %eax
	movl	%eax, 20(%rsp)
	jmp	.LBB0_10
.LBB0_2:                                # %bb
                                        #   in Loop: Header=BB0_1 Depth=1
	callq	rand
	movl	24(%rsp), %ecx
	cltd
	idivl	%ecx
	addl	$-10, %ecx
	cmpl	%ecx, %edx
	jg	.LBB0_12
# BB#3:                                 # %bb2
                                        #   in Loop: Header=BB0_1 Depth=1
	movl	$10, 12(%rsp)
	callq	rand
	movl	24(%rsp), %ecx
	cltd
	idivl	%ecx
	addl	$-10, %ecx
	cmpl	%ecx, %edx
	jg	.LBB0_14
# BB#4:                                 # %bb5
                                        #   in Loop: Header=BB0_1 Depth=1
	movl	$10, 12(%rsp)
	callq	rand
	movl	24(%rsp), %ecx
	cltd
	idivl	%ecx
	addl	$-10, %ecx
	cmpl	%ecx, %edx
	jg	.LBB0_17
# BB#5:                                 # %bb8
                                        #   in Loop: Header=BB0_1 Depth=1
	movl	$10, 12(%rsp)
	callq	rand
	movl	24(%rsp), %ecx
	cltd
	idivl	%ecx
	addl	$-10, %ecx
	cmpl	%ecx, %edx
	jg	.LBB0_20
# BB#6:                                 # %bb11
                                        #   in Loop: Header=BB0_1 Depth=1
	movl	$10, 12(%rsp)
	callq	rand
	movl	24(%rsp), %ecx
	cltd
	idivl	%ecx
	addl	$-10, %ecx
	cmpl	%ecx, %edx
	jg	.LBB0_23
# BB#7:                                 # %bb14
                                        #   in Loop: Header=BB0_1 Depth=1
	movl	$10, 12(%rsp)
	callq	rand
	movl	24(%rsp), %ecx
	cltd
	idivl	%ecx
	addl	$-10, %ecx
	cmpl	%ecx, %edx
	jg	.LBB0_26
# BB#8:                                 # %bb17
                                        #   in Loop: Header=BB0_1 Depth=1
	movl	$10, 12(%rsp)
	callq	rand
	movl	24(%rsp), %ecx
	cltd
	idivl	%ecx
	addl	$-10, %ecx
	cmpl	%ecx, %edx
	jg	.LBB0_29
# BB#9:                                 # %bb20
                                        #   in Loop: Header=BB0_1 Depth=1
	movl	$10, 12(%rsp)
	movl	$20, 20(%rsp)
.LBB0_10:                               # %bb20
                                        #   in Loop: Header=BB0_1 Depth=1
	movl	16(%rsp), %eax
	addl	%eax, 8(%rsp)
	incl	16(%rsp)
.LBB0_1:                                # %bb22
                                        # =>This Inner Loop Header: Depth=1
	movl	16(%rsp), %eax
	cmpl	28(%rsp), %eax
	jl	.LBB0_2
# BB#11:                                # %bb23
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
