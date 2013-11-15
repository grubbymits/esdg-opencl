int work_dim = 1;
int global_size[] = { 256, 0, 0 };
int local_size[] = { 256, 0, 0 };
int num_groups[] = { 1, 0, 0 };
int global_offset[] = { 0, 0, 0 };
int num_cores = 1;
int group_id = 0;
int result = 0;

float add(float a, float b) {
  return a + b;
}

int main(void) {
  float a = 0.5;
  float b = 0.5;
  result = (int)add(a, b);
  if (result == 1)
    return 1;
  else
    return 0;
}


