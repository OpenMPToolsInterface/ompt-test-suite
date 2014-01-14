#include <omp.h>

#include "fib.h"

#define N 42

void g()
{
#pragma omp parallel 
{
    int val = N;
    int n = omp_get_thread_num();

    /* adjust val for imbalanced waiting */
    if (n < 42) val = val-n;

    fib(val);
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
