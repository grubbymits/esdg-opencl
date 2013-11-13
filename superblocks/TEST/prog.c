#include <stdio.h>

int main() {
  int i = 0;
  int ans = 0;
  for (i = 0; i < 50; i++) {
    if (i < 45) {
      ans += 3;
    } else {
      ans += 2;
    }

    ans += 10;
  }

  return 0;
}
