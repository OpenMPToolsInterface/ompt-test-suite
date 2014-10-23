#include <stdio.h>
#include <unistd.h>
#include "omp.h"

#define SIZE 10000
#include "callback.h"

#ifndef USE_OFFLOAD
#define USE_OFFLOAD 1
#endif

// This program calculates a reduction of vector x using liboffload.
int main(void) {

	double* x;
	double sum;
	int     i, j;

	x = (double*) malloc(sizeof(double) * SIZE);

	// initialize vector which has to be reduced
	for (i = 0; i < SIZE; i++) {
		x[i] = 1.0;
	}

	#pragma omp target map(to: x[0:SIZE]) map(sum) 
	{
		#ifdef __MIC__
			printf("Running on MIC...\n");
			fflush(0);
		#else
			printf("Running on HOST...\n");
		#endif	
	
		int i;
		#pragma omp parallel for reduction(+:sum)
		for(i = 0; i < SIZE; i++) {
			sum = sum + x[i];
		}
	}

	// result should be SIZE 
	printf("Result: sum = %.2le\n", sum);
	if (sum != SIZE) {
		printf("ERROR: Result of reduction not correct (expected: %.2le\n)", (double) SIZE);
	}

	return 0;

}
