#include <omp.h>

#include "fib.h"

#define N 42

void g()
{
#pragma omp parallel 
{
    if (omp_get_thread_num() % 2  == 0) {
      fib(N);
    }
    else {
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
