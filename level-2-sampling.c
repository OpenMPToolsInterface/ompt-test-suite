#include <stdio.h>
#include <omp.h>
#include <ompt.h>
#include "fib.h"
#define MAX (1LL << 28)
main()
{ 
  long long i,j,k;
  omp_set_nested(1);
  #pragma omp parallel 
  {
    for(i = 0; i < MAX; i++);
    #pragma omp parallel 
    {
      for(j = 0; j < MAX; j++);
      fib(40);
    }
  }
}
