#include <stdio.h>
#include <unistd.h>
#include "omp.h"
#include "callback.h"

#define SIZE 1024 // should be a power of 2
#define THREAD_NUM 2 // should be a power of 2
#define ALPHA 2

#ifndef USE_OFFLOAD
#define USE_OFFLOAD 1 
#endif

// This test checks the update directive on two devices by using THREAD_NUM threads.
// The first half of the threads works on device 0, the second on device 1.
int main() {
	
	int* x = (int*) malloc(SIZE * sizeof(int));

	// initialize array with same value
	int i;
	for (i = 0; i < SIZE; i++) {
		x[i] = ALPHA;		
	}

	int elem_per_thread = SIZE / THREAD_NUM;
	printf("\n*** Values per thread: %d ***\n\n", elem_per_thread);
 
	int task;
	#pragma omp target data map(alloc: x[0:SIZE]) device(0)
	#pragma omp target data map(alloc: x[0:SIZE]) device(1)
	#pragma omp parallel num_threads(THREAD_NUM)
	{
		// decide if this thread has to run on device 0 or 1
		int thread = omp_get_thread_num();

		int device_id;
		if (thread < THREAD_NUM/2) {
			device_id = 0;
		} else {
			device_id = 1;
		}
		
		// copy (only needed) values to device
		#pragma omp target update to(x[thread*elem_per_thread: elem_per_thread]) device(device_id)

		// modify values (add i to every value)
		#pragma omp target device(device_id)
		{
			int i;
			#pragma omp parallel for
			for(i = thread * elem_per_thread; i < (thread + 1) * elem_per_thread; i++) {
				x[i] += i;
			}
		}

		// copy modified values back to host
		#pragma omp target update from(x[thread*elem_per_thread: elem_per_thread]) device(device_id)
	}


	// check correctness: x[i] = ALPHA + i for all i
	for (i = 0; i < SIZE; i++) {
		if (x[i] != ALPHA + i) {
			printf("ERROR: x[%d] = %d, expected x[%d] = %d", i, x[i], i, ALPHA + i);
			break;
		}
	}

}
