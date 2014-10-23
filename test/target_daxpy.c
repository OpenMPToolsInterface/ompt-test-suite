#include <stdio.h>
#include <unistd.h>
#include "omp.h"

#define SIZE 10000
#define ALPHA 2
#include "callback.h"

#ifndef USE_OFFLOAD
#define USE_OFFLOAD 1 
#endif

int main(){
	
	double   a;
	double*  x;
	double*  y;
	int      i;
	int  first;

	x = (double*)malloc(sizeof(double)*SIZE);
	y = (double*)malloc(sizeof(double)*SIZE);

	a = ALPHA;

	// init in a way that the result is 1
	for (i = 0; i<SIZE; i++){
		x[i] = (double)i;
		y[i] = 1.0 - a * x[i];
	}

	#pragma omp target map(to:    x[0:SIZE]) \
	                   map(tofrom:y[0:SIZE]) \
	                       if(USE_OFFLOAD)
	{
		#ifdef __MIC__
		printf("Hello MIC\n");
		fflush(0);
		#else	
		printf("Hello HOST\n");
		#endif

		int i;
		#pragma omp parallel for
		for(i=0; i<SIZE; i++){
			y[i] = a * x[i] + y[i];
		}
	}	

	// print first 10 and last 10 results of y
	first = 1;
	printf("Result: y = (");
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
