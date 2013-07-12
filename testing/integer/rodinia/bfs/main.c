extern int BufferArg_0;
extern int BufferArg_1;
extern int BufferArg_2;
extern int BufferArg_3;
int main(void) {
  for (unsigned z = 0; z < 1; ++z) {
__builtin_le1_set_group_id_2(z);
      for (unsigned x = 0; x < 1; ++x) {
        __builtin_le1_set_group_id_0(x);
        BFS_2(&BufferArg_0, &BufferArg_1, &BufferArg_2, &BufferArg_3, 4096);
}
}
return 0;
}