volatile unsigned a;
volatile unsigned b;

int NANDL(unsigned s1, unsigned s2) {
  return ((((s1) == 0) | ((s2) == 0)) ? 1 : 0);
}

int NORL(unsigned s1, unsigned s2) {
  return ((((s1) == 0) & ((s2) == 0)) ? 1 : 0);
}

int ORL(unsigned s1, unsigned s2) {
  return ((((s1) == 0) & ((s2) == 0)) ? 0 : 1);
}

int ANDL(unsigned s1, unsigned s2) {
  return (((((s1) == 0) | ((s2) == 0)) ? 0 : 1));
}
int TBIT(unsigned s1, unsigned s2) {
  return (((s1) & ((unsigned int) 1 << (s2))) ? 1 : 0);
}

int TBITF(unsigned s1, unsigned s2) {
  return (((s1) & ((unsigned int) 1 << (s2))) ? 0 : 1);
}

int SBIT(unsigned s1, unsigned s2) {
  return ((s1) | ((unsigned int) 1 << (s2)));
}

int SBITF(unsigned s1, unsigned s2) {
  return ((s1) & ~((unsigned int) 1 << (s2)));
}

int main(void) {
  return NANDL(a, b);
}
