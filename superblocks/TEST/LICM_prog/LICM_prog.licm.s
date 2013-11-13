	.file	"<stdin>"
	.text
	.globl	main
	.align	16, 0x90
	.type	main,@function
main:                                   # @main
# BB#0:                                 # %entry
	subq	$24, %rsp
	movl	$0, 12(%rsp)
	movl	$0, 8(%rsp)
	movl	$0, 4(%rsp)
	movl	$0, 12(%rsp)
	jmp	.LBB0_1
	.align	16, 0x90
.LBB0_6:                                # %bb1
                                        #   in Loop: Header=BB0_1 Depth=1
	movl	12(%rsp), %eax
	movl	%eax, 8(%rsp)
	jmp	.LBB0_4
.LBB0_2:                                # %bb
                                        #   in Loop: Header=BB0_1 Depth=1
	cmpb	$0, 12(%rsp)
	je	.LBB0_6
# BB#3:                                 # %bb2
                                        #   in Loop: Header=BB0_1 Depth=1
	movl	8(%rsp), %eax
.LBB0_4:                                # %bb2
                                        #   in Loop: Header=BB0_1 Depth=1
	leal	-4(,%rax,4), %eax
	movl	%eax, 4(%rsp)
	incl	12(%rsp)
.LBB0_1:                                # %bb3
                                        # =>This Inner Loop Header: Depth=1
	cmpl	$99999999, 12(%rsp)     # imm = 0x5F5E0FF
	jle	.LBB0_2
# BB#5:                                 # %bb4
	movl	4(%rsp), %esi
	movl	$.L.str, %edi
	xorb	%al, %al
	callq	printf
	movl	$0, 16(%rsp)
	movl	$0, 20(%rsp)
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


	.section	".note.GNU-stack","",@progbits
