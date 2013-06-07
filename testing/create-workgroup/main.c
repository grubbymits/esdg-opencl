int work_dim = 2;
int global_size[3] = {1024, 1024};
int local_size[3] = {256, 256};
int num_groups[3] = {4, 4};
int global_offset[3] = {0, 0, 0};
int main(void) {
  vector_mult(0x0, 0x10, 0x20);
}
