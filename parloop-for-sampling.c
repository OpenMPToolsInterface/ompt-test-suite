#include <omp.h>

#include "fib.h"

#define N 18

void g()
{
  int i, j;
for(j = 0; j< 50000; j++) {
#pragma omp parallel for
  for(i = 0; i<5; i++) {
    fib(N);
  }
}
}

void f()
{
  g();
}

int main()
{
  f();
  return 0;
}
