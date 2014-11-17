#include <stdio.h>
#include <unistd.h>
#include "omp.h"
#include "callback.h"

#define SIZE 10000
#define ALPHA 2

#ifndef USE_OFFLOAD
#define USE_OFFLOAD 1 
#endif

int main() {
	
	int* x = (int*) malloc(SIZE * sizeof(int));
	int* y = (int*) malloc(SIZE * sizeof(int));

	// initialize both arrays with same value
	int i;
	for (i = 0; i < SIZE; i++) {
		x[i] = ALPHA;		
		y[i] = ALPHA;
	}

	printf("\n*** Entering data region... ***\n");
	#pragma omp target data map(to: x[0:SIZE/2]) if(USE_OFFLOAD)
	{
		printf("\n*** Modifying first half of x on device... ***\n");
		#pragma omp target
		{
			#ifdef __MIC__
				printf("Running on MIC...\n");
				fflush(0);
			#else
				printf("Running on HOST...\n");
			#endif
	
			int i;
			// modify first half of array on device
			#pragma omp parallel for
			for (i = 0; i < SIZE/2; i++) {
				x[i] += 1;
			}
		}

		printf("\n*** Copying modified half of x back to host... ***\n");
		// copy modified array half explicitly to host
		#pragma omp target update from(x[0:SIZE/2])

		int i;
		for (i = 0; i < SIZE/2; i++) {
			if (x[i] != ALPHA + 1) {
				printf("ERROR: Update from device to host not working\n");
				printf("x[%d] = %d, expected: %d\n", i, x[i], ALPHA + 1);
				break;
			}
		}		

		// other half should not be modified
		for (i = SIZE/2; i < SIZE; i++) {
			if (x[i] != ALPHA) {
				printf("ERROR: Update modified values which should not change");
				printf("x[%d] = %d, expected: %d\n", i, x[i], ALPHA);
				break;
			}
		}

		printf("\n*** Entering nested data region... ***\n");
		#pragma omp target data map(alloc: y[0:SIZE]) if(USE_OFFLOAD)
		{
			printf("\n*** Copying y to device... ***\n");
			// copy values manually to device
			#pragma omp target update to(y[0:SIZE])

			printf("\n*** Modifying y on device... ***\n");
			#pragma omp target
			{
				#ifdef __MIC__
					printf("Running on MIC...\n");
					fflush(0);
				#else
					printf("Running on HOST...\n");
				#endif

				int i;
				// check if update worked correctly
				for (i = 0; i < SIZE; i++) {
					if (y[i] != ALPHA) {
						printf("ERROR: Update from host to device not wokring.");
						printf("y[%d] = %d, expected: %d\n", i, y[i], ALPHA);
						break;
					}
				}

				// modify array
				#pragma omp parallel for
				for (i = 0; i < SIZE; i++) {
					y[i] += 1;
				}	
			}
		
			printf("\n*** Copying y back to host... ***\n");

			// copy modified values back to host
			#pragma omp target update from(y[0:SIZE])

			// check if update to host worked correctly
			int i;
			for (i = 0; i < SIZE; i++) {
				if (y[i] != ALPHA + 1) {
					printf("ERROR: Update from device to host not working\n");
					printf("y[%d] = %d, expected: %d\n", i, y[i], ALPHA + 1);
					break;
				}
			}
			
			printf("\n*** End of nested region... ***\n");
		}


		printf("\n*** End of data region... ***\n");
	}

}
