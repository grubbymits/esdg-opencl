## Operations - 142
-- FUNC_main
sub.0 r0.1, r0.1, 0x18
;
stw.0 r0.1[0x14], l0.0
stw.0 r0.1[0x10], r0.0
;
stw.0 r0.1[0xc], r0.0
goto.0 (temp_BFS_1_L0?1)
;
-- temp_BFS_1_L0?8
ldw.0 r0.2, r0.1[0xc]
;
add.0 r0.2, r0.2, 0x1
;
stw.0 r0.1[0xc], r0.2
goto.0 (temp_BFS_1_L0?1)
;
-- temp_BFS_1_L0?1
ldw.0 r0.2, r0.1[0xc]
;
cmpne.0 b0.0, r0.2, r0.0
;
br.0 b0.0, (temp_BFS_1_L0?9)
;
goto.0 (temp_BFS_1_L0?2)
;
-- temp_BFS_1_L0?2
ldw.0 r0.2, r0.1[0xc]
;
stw.0 r0.0[(group_id+8)], r0.2
stw.0 r0.1[0x8], r0.0
;
goto.0 (temp_BFS_1_L0?3)
;
-- temp_BFS_1_L0?7
ldw.0 r0.2, r0.1[0x8]
;
add.0 r0.2, r0.2, 0x1
;
stw.0 r0.1[0x8], r0.2
goto.0 (temp_BFS_1_L0?3)
;
-- temp_BFS_1_L0?3
mtbf.0 b0.0, r0.0
;
br.0 b0.0, (temp_BFS_1_L0?8)
;
goto.0 (temp_BFS_1_L0?4)
;
-- temp_BFS_1_L0?4
ldw.0 r0.2, r0.1[0x8]
;
stw.0 r0.0[(group_id+4)], r0.2
stw.0 r0.1[0x4], r0.0
;
goto.0 (temp_BFS_1_L0?5)
;
-- temp_BFS_1_L0?6
ldw.0 r0.2, r0.1[0x4]
;
stw.0 r0.0[(group_id+0)], r0.2
mov.0 r0.3, BufferArg_0
;
mov.0 r0.4, BufferArg_1
mov.0 r0.5, BufferArg_2
;
mov.0 r0.6, BufferArg_3
mov.0 r0.7, BufferArg_4
;
mov.0 r0.8, BufferArg_5
mov.0 r0.9, 0x1000
;
call.0 l0.0, FUNC_BFS_1
;
ldw.0 r0.2, r0.1[0x4]
;
add.0 r0.2, r0.2, 0x1
;
stw.0 r0.1[0x4], r0.2
goto.0 (temp_BFS_1_L0?5)
;
-- temp_BFS_1_L0?5
ldw.0 r0.2, r0.1[0x4]
;
cmpgtu.0 b0.0, r0.2, 0x7
;
br.0 b0.0, (temp_BFS_1_L0?7)
;
goto.0 (temp_BFS_1_L0?6)
;
-- temp_BFS_1_L0?9
mov.0 r0.3, r0.0
ldw.0 l0.0, r0.1[0x14]
;
.call exit, caller, arg($r0.3:s32), ret($r0.3:s32)
call.0 l0.0, FUNC_exit
;
-- FUNC_BFS_1
mov.0 r0.2, r0.0
mov.0 r0.10, 0x1
;
mov.0 r0.11, r0.0
;
-- temp_BFS_1_L1?1
ldw.0 r0.13, r0.0[(group_id+0)]
ldw.0 r0.14, r0.0[(num_cores+0)]
;
mpylu.0 r0.12, r0.13, r0.14
mpyhs.0 r0.13, r0.13, r0.14
;
cpuid.0 r0.14
add.0 r0.12, r0.12, r0.13
;
shru.0 r0.13, r0.14, 0x8
;
add.0 r0.12, r0.12, r0.13
ldw.0 r0.13, r0.0[(local_size+0)]
;
mpylu.0 r0.14, r0.12, r0.13
mpyhs.0 r0.12, r0.12, r0.13
;
add.0 r0.12, r0.14, r0.12
;
add.0 r0.12, r0.12, r0.11
;
cmpge.0 b0.0, r0.12, r0.9
;
br.0 b0.0, (temp_BFS_1_L1?8)
;
goto.0 (temp_BFS_1_L1?2)
;
-- temp_BFS_1_L1?2
add.0 r0.14, r0.5, r0.12
;
ldbu.0 r0.13, r0.14[0x0]
;
cmpeq.0 b0.0, r0.13, r0.0
;
br.0 b0.0, (temp_BFS_1_L1?8)
;
goto.0 (temp_BFS_1_L1?3)
;
-- temp_BFS_1_L1?3
sh3add.0 r0.13, r0.12, r0.3
stb.0 r0.14[0x0], r0.2
;
ldw.0 r0.15, r0.13[0x4]
;
cmplt.0 b0.0, r0.15, 0x1
;
br.0 b0.0, (temp_BFS_1_L1?8)
;
goto.0 (temp_BFS_1_L1?4)
;
-- temp_BFS_1_L1?4
ldw.0 r0.16, r0.13[0x0]
;
mov.0 r0.14, r0.16
;
-- temp_BFS_1_L1?5
sh2add.0 r0.17, r0.14, r0.4
;
ldw.0 r0.17, r0.17[0x0]
;
add.0 r0.18, r0.7, r0.17
;
ldbu.0 r0.18, r0.18[0x0]
;
cmpne.0 b0.0, r0.18, r0.0
;
br.0 b0.0, (temp_BFS_1_L1?7)
;
goto.0 (temp_BFS_1_L1?6)
;
-- temp_BFS_1_L1?6
sh2add.0 r0.15, r0.12, r0.8
;
ldw.0 r0.15, r0.15[0x0]
;
add.0 r0.15, r0.15, 0x1
sh2add.0 r0.16, r0.17, r0.8
;
stw.0 r0.16[0x0], r0.15
add.0 r0.15, r0.6, r0.17
;
stb.0 r0.15[0x0], r0.10
ldw.0 r0.16, r0.13[0x0]
;
ldw.0 r0.15, r0.13[0x4]
;
-- temp_BFS_1_L1?7
add.0 r0.14, r0.14, 0x1
add.0 r0.17, r0.16, r0.15
;
cmplt.0 b0.0, r0.14, r0.17
;
br.0 b0.0, (temp_BFS_1_L1?5)
;
goto.0 (temp_BFS_1_L1?8)
;
-- temp_BFS_1_L1?8
add.0 r0.11, r0.11, 0x1
;
cmpne.0 b0.0, r0.11, 0x100
;
br.0 b0.0, (temp_BFS_1_L1?1)
;
goto.0 (temp_BFS_1_L1?9)
;
-- temp_BFS_1_L1?9
return.0 r0.1, r0.1, 0x0, l0.0
;
-- FUNC_BFS_2
mov.0 r0.2, r0.0
mov.0 r0.8, 0x1
;
mov.0 r0.9, r0.0
;
-- temp_BFS_1_L2?1
ldw.0 r0.11, r0.0[(group_id+0)]
ldw.0 r0.12, r0.0[(num_cores+0)]
;
mpylu.0 r0.10, r0.11, r0.12
mpyhs.0 r0.11, r0.11, r0.12
;
cpuid.0 r0.12
add.0 r0.10, r0.10, r0.11
;
shru.0 r0.11, r0.12, 0x8
ldw.0 r0.12, r0.0[(local_size+0)]
;
add.0 r0.11, r0.11, r0.10
;
mpylu.0 r0.10, r0.12, r0.11
mpyhs.0 r0.11, r0.12, r0.11
;
add.0 r0.10, r0.10, r0.11
;
add.0 r0.10, r0.9, r0.10
;
cmpge.0 b0.0, r0.10, r0.7
;
br.0 b0.0, (temp_BFS_1_L2?4)
;
goto.0 (temp_BFS_1_L2?2)
;
-- temp_BFS_1_L2?2
add.0 r0.11, r0.4, r0.10
;
ldbu.0 r0.12, r0.11[0x0]
;
cmpeq.0 b0.0, r0.12, r0.0
;
br.0 b0.0, (temp_BFS_1_L2?4)
;
goto.0 (temp_BFS_1_L2?3)
;
-- temp_BFS_1_L2?3
add.0 r0.12, r0.3, r0.10
;
stb.0 r0.12[0x0], r0.8
add.0 r0.10, r0.5, r0.10
;
stb.0 r0.10[0x0], r0.8
stb.0 r0.6[0x0], r0.8
;
stb.0 r0.11[0x0], r0.2
;
-- temp_BFS_1_L2?4
add.0 r0.9, r0.9, 0x1
;
cmpne.0 b0.0, r0.9, 0x100
;
br.0 b0.0, (temp_BFS_1_L2?1)
;
goto.0 (temp_BFS_1_L2?5)
;
-- temp_BFS_1_L2?5
return.0 r0.1, r0.1, 0x0, l0.0
;

