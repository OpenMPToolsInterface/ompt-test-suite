#define __STDC_FORMAT_MACROS

#include <inttypes.h>
#include <stdio.h>
#include <omp.h>

#define NTHREADS 8

uint64_t fib(int n) 
{
  if (n < 2) return n;
  else return fib(n-1) + fib(n-2);
}

int main(int argc, char **argv)
{
	int i;
	uint64_t *result = new uint64_t[NTHREADS]; 
#pragma omp parallel num_threads(NTHREADS) 
	{
		int tid = omp_get_thread_num();
		result[tid] = fib(40-tid);
	}
	for(i=0;i<NTHREADS;i++) {
		printf("%" PRIu64 " ", result[i]);
	}
	printf("\n");
}
