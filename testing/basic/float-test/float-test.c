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


