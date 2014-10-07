#include <omp.h>

#include "fib.h"

#define N 42

void g()
{
#pragma omp parallel 
{
    fib(N);
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
