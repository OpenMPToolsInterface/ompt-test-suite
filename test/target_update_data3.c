#include <stdio.h>
#include <unistd.h>
#include "omp.h"
#include "callback.h"

#define SIZE 10000

#ifndef USE_OFFLOAD
#define USE_OFFLOAD 1 
#endif

// This program uses the update directive for transferring data.
int main(){
	
	int i;
	int a;
	double x_a_old;
	double* x;

	x = (double*)malloc(sizeof(double)*SIZE);

	// start values on host	
	a = -1;
	for (i = 0; i<SIZE; i++){
		x[i] = 1.0;
	}

	#pragma omp target data map(alloc: a, x_a_old) map(to: x[0:SIZE]) if(USE_OFFLOAD)
	{
		#pragma omp target
		{
			#ifdef __MIC__
				printf("Running on MIC...\n");
				fflush(0);
				#else
				printf("Running on HOST...\n");
			#endif
			
			a = 42;
	
			// modify a on device
			#pragma omp parallel for
			for(i=0; i<SIZE; i++)
				x[i] += a;
			
			x_a_old = x[a];

		}

		// copy new values to host
		#pragma omp target update from(a)
		#pragma omp target update from(x[0:SIZE])

		printf("HOST: a = %d, x_%d = %f\n", a, a, x[a]);

		if (x[a] == 1.0 || a == -1) {
			printf("ERROR: Update from device to host not working");
		}

		// modify a on host
		x[a] = 0.0;
	
		// copy new value to device
		#pragma omp target update to(x[0:SIZE])
		#pragma omp target
		{
			if (x_a_old == x[a]) {
				printf("ERROR: Update from device to host not working");
			}

			printf("DEVICE: x_a_old = %f, x_%d = %f\n", x_a_old, a, x[a]);			
		}

	}

	return 0;	
}
