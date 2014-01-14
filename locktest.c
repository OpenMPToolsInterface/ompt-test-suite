#include <omp.h>

#include "fib.h"

#define N 10

void g()
{
  int i, j;
  omp_lock_t l;
  omp_init_lock(&l); 

#pragma omp parallel for
  for(i = 0; i<10000000; i++) {
    omp_set_lock(&l); 
    fib(N);
    omp_unset_lock(&l); 
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
