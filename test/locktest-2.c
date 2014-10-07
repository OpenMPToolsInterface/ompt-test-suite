#include <omp.h>
#include "fib.h"
void g() {
  int i;
  omp_lock_t l;
  omp_init_lock(&l); 
  #pragma omp parallel 
  {
    #pragma omp master
    {
      omp_set_lock(&l); 
      fib(40);
      omp_unset_lock(&l); 
    }
    #pragma omp for
    for(i = 0; i<100; i++) {
      omp_set_lock(&l); 
      fib(10);
      omp_unset_lock(&l); 
    }
  }
}
void f() { g(); }
int main() { f(); return 0; }
