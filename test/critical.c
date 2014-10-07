#include <omp.h>
#include "fib.h"
void g()
{
  int i;
  omp_lock_t l;
  omp_init_lock(&l); 
#pragma omp parallel 
  {
    #pragma omp master
    {
      #pragma omp critical (it)
      { fib(40); }
    }
    #pragma omp for
    for(i = 0; i<100; i++) {
      #pragma omp critical (it)
      { fib(10); }
    }
  }
}
void f() { g(); }
int main() { f(); return 0; }
