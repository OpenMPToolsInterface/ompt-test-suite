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
	double* x;

	x = (double*)malloc(sizeof(double)*SIZE);

	for (i = 0; i<SIZE; i++){
		x[i] = 1.0;
	}


	// start value on host	
	a = 1;

	// save old values for error checking
	int a_old_device, a_old_host;

	#pragma omp target data map(alloc: a, a_old_device) map(to: x[0:SIZE]) if(USE_OFFLOAD)
	{
		#pragma omp target
		{
			#ifdef __MIC__
				printf("Running on MIC...\n");
				fflush(0);
				#else
				printf("Running on HOST...\n");
			#endif
			
			// modify a on device
			#pragma omp parallel for reduction(+:a)
			for(i=0; i<SIZE; i++)
				a += x[i];

			// save modified a on device
			a_old_device = a;
		}

		// save old a on host
		a_old_host = a;

		// copy new value to host
		#pragma omp target update from(a)

		printf("HOST: a_old_host = %d, a = %d\n", a_old_host, a);

		if (a_old_host == a) {
			printf("ERROR: Update from device to host not working");
		}

		// modify a on host
		a = 0;
	
		// copy new value to device
		#pragma omp target update to(a)
		#pragma omp target
		{
			if (a_old_device == a) {
				printf("ERROR: Update from device to host not working");
			}

			printf("DEVICE: a_old_device = %d, a = %d\n", a_old_device, a);			
		}

	}

	return 0;	
}
