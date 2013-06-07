	.file	"data.o"
	.text
	.globl	main
	.align	2
	.type	main,@function
main:                                   # @main
# BB#0:                                 # %entry
	sub.0  r0.1, r0.1, 0x8
	;;
	stw.0  r0.1[0x4], r0.0
	;;
	mov.0  r0.2, r0.0
	;;
	mov.0  r0.3, r0.0
	;;
	call.0  l0.0, exit
	;;
	return.0  r0.1, r0.1, 0x0, l0.0
	;;
$tmp0:
	.size	main, ($tmp0)-main

	.type	bytes,@object           # @bytes
	.data
	.globl	bytes
bytes:
	.ascii	 "\000\001\002\003\004\005\006\007\b\t"
	.size	bytes, 10

	.type	halves,@object          # @halves
	.section	.sdata,"aw",@progbits
	.globl	halves
	.align	1
halves:
	.2byte	100                     # 0x64
	.2byte	1000                    # 0x3e8
	.2byte	1023                    # 0x3ff
	.2byte	10000                   # 0x2710
	.size	halves, 8

	.type	words,@object           # @words
	.globl	words
	.align	2
words:
	.4byte	65300                   # 0xff14
	.4byte	1512                    # 0x5e8
	.size	words, 8


