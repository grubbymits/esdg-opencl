extern int BufferArg_0;
extern int BufferArg_1;
extern int BufferArg_2;
extern int BufferArg_3;
extern int BufferArg_4;
extern int BufferArg_5;
extern unsigned char group_id[8];

int main(void) {
  for (unsigned z = 0; z < 1; ++z) {
group_id[((__builtin_le1_read_cpuid()*4) + 2)] = z;
      for (unsigned x = 0; x < 8; ++x) {
        group_id[((__builtin_le1_read_cpuid()*4) + 0)] = x;
        BFS_1(&BufferArg_0, &BufferArg_1, &BufferArg_2, &BufferArg_3, &BufferArg_4, &BufferArg_5, 4096);
}
}
return 0;
}