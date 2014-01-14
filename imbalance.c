#include <omp.h>
#include <stdio.h>
//#include "callback.h"

#include "fib.h"

int main()
{
  int i;
  int num = 24;
#if 0
  int num = omp_get_num_procs();
  printf("num threads = %d\n", num); 
#endif
  #pragma omp parallel shared(i,num)
  {
    #pragma omp for nowait
      for(i=0; i<num;i++) {
        if(i%2 == 0)
	  fib(42);
	else
	  fib(40);
      }
    #pragma omp for
      for(i=0; i<num;i++) {
	  fib(40);
      }
  }
  return 0;
}
