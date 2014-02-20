volatile unsigned a = 10;
volatile unsigned b = 20;

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

int ANDC(unsigned s1, unsigned s2) {
  return ((~s1 & s2));
}

int ORC(unsigned s1, unsigned s2) {
  return (~s1 | s2);
}

typedef unsigned short ui16;
typedef short i16;

int MULHS(int s1, int s2) {
  return (s1 * ((ui16)(s2 >> 16))) << 16;
}

int MULH(int s1, int s2) {
  return s1 * (i16)(s2 >> 16);
}

int MULHU(int s1, int s2) {
  return s1 * (ui16)(s2 >> 16);
}

int MULLU(int s1, int s2) {
  return s1 * (ui16)s2;
}
int MULL(int s1, int s2) {
  return s1 * (i16)s2;
}
int MULLHU(int s1, int s2) {
  return ((ui16)(s1)) * ((ui16)(s2 >> 16));
}

int MULHHU(int s1, int s2) {
  return ((ui16)(s1 >> 16) * (ui16)(s2 >> 16));
}

int MULLLU(int s1, int s2) {
  return ((ui16)(s1) * (ui16)(s2));
}

int MULLL(int s1, int s2) {
  return ((i16)(s1) * (i16)(s2));
}

int  MULHH(int s1, int s2) {
  return ((i16)(s1 >> 16) * (i16)(s2 >> 16));
}

int MULLH(int s1, int s2) {
  return (i16)(s1) * (i16)(s2 >> 16);
}

int main(void) {
  int c = NANDL(a, b);
  return c;
}
