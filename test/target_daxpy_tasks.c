#include <stdio.h>
#include <unistd.h>
#include "omp.h"

#define SIZE 10000 
#define ALPHA 5
#include "callback.h"

#ifndef USE_OFFLOAD
#define USE_OFFLOAD 1
#endif

// This test program calculates y = a*x + y with tasks using liboffload.
// The result values should all be 1.
int main(void) {

	double* x = (double*) malloc(sizeof(double*) * SIZE);
	double* y = (double*) malloc(sizeof(double*) * SIZE);
	double a = ALPHA;
	int i, first;

	// initialize arrays, that all result values are 1
	for (i = 0; i < SIZE; i++) {
		x[i] = (double) i; 
		y[i] = 1.0 - a * x[i];
	}	

	// calculate daxpy with tasks 
	#pragma omp target map(to:    x[0:SIZE]) \
                           map(tofrom: y[0:SIZE]) \
				if(USE_OFFLOAD)
	#pragma omp parallel
	#pragma omp single
	{	
		#ifdef __MIC__
			printf("Running on MIC...\n");
			fflush(0);
		#else
			printf("Running on HOST...\n");
		#endif

		// create own task for every entry 
		int i;
		for(i = 0; i < SIZE; i++) {
			#pragma omp task firstprivate(i)
			{
				// calculate y = a*x + y  
				y[i] = a * x[i] + y[i];
			}
		}
		#pragma omp taskwait
	}
	
	printf("Result: y = (");
	first = 1;
	// print first 10 and last 10 results of y
	for (i=0; i<SIZE; i++){
		if(i < 10 || (i> SIZE - 10) && i<SIZE-1){
			printf("%.2le, ", y[i]);
		} else if (i == SIZE-1) {
			printf("%.2le", y[i]);
		} else if(first){ 
			printf("..., ");
			first = 0;
		}	
	}
	printf(")\n");

	for (i=0; i<SIZE; i++){
		if(y[i]!=1.0){
			printf("ERROR: Result is not correct for y_%ld\n", i);
			return 1;
		}
	}

	return 0;
}


