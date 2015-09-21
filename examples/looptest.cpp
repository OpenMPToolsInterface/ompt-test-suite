#define __STDC_FORMAT_MACROS

#include <inttypes.h>
#include <stdio.h>
#include <omp.h>

#define NTHREADS 8
#define NBOUND 40 

uint64_t result[NBOUND]; 

uint64_t fib(int n) 
{
  if (n < 2) return n;
  else return fib(n-1) + fib(n-2);
}

void testparallel()
{
#pragma omp parallel num_threads(NTHREADS) 
  {
#pragma omp for
    for(int i = 0; i < NBOUND; i++)  result[i] = fib(40);
  }
}

void teststatic()
{
#pragma omp parallel for schedule(static)
  for(int i = 0; i < NBOUND; i++) result[i] += fib(40);
}


void testdynamic()
{
#pragma omp parallel for schedule(dynamic)
  for(int i = 0; i < NBOUND; i++) result[i] += fib(40);
}


void testguided()
{
#pragma omp parallel for schedule(guided)
  for(int i = 0; i < NBOUND; i++) result[i] += fib(40);
}


int main(int argc, char **argv)
{
  testparallel();
  teststatic();
  testdynamic();
  testguided();

  for(int i=0;i< NBOUND;i++) printf("%" PRIu64 " ", result[i]);
  printf("\n");
}
