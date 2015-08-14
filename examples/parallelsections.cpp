#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdio.h>
#include <omp.h>

long fib(long n) 
{
  if (n < 2) return n;
  else return fib(n-1) + fib(n-2);
}

long sleep(int n)
{
  return fib(40);
}


 
int main (void)
{
   omp_set_nested(1);
   omp_set_max_active_levels(8);
   omp_set_dynamic(0);
 
   for (volatile int i=0; i<100000000; ++i)
      i = i ;
 
   omp_set_num_threads(2);
   #pragma omp parallel sections
   {
      #pragma omp section
      {
         omp_set_num_threads(5);
         #pragma omp parallel for
         for (int j=0; j<5; ++j) {
            sleep(1) ;
            printf("wake %d, %d\n", j, omp_get_thread_num()) ;
         }
      }
 
      #pragma omp section
      {
         omp_set_num_threads(3);
         #pragma omp parallel for
         for (int j=0; j<3; ++j) {
            sleep(1) ;
            printf("wake %d, %d\n", j, 5 + omp_get_thread_num()) ;
         }
      }
   }
 
   for (volatile int i=0; i<100000000; ++i)
      i = i ;
 
   return 0;
}
