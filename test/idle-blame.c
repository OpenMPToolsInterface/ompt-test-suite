#include <omp.h>

#include "fib.h"

#define N 42

// every other thread works. blame from the odd threads
void g()
{
#pragma omp parallel 
{
    if (omp_get_thread_num() % 2  == 0) {
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
