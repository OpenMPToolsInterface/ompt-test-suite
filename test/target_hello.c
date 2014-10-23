#include <stdio.h>
#include <unistd.h>
#include <omp.h>

#include "callback.h"

#ifndef USE_OFFLOAD
#define USE_OFFLOAD 1
#endif

int main(){
	#pragma omp parallel
	{
		printf("I am host tread %d\n", omp_get_thread_num());
	}

	#pragma omp target if(USE_OFFLOAD)
	{
		#ifdef __MIC__
		printf("Hello MIC\n");
		#pragma omp parallel 
		{
			int i;
		  char hostname[100];
  		gethostname(hostname, 100);

			#pragma omp critical
			printf("I am mic thread %d (%s)\n", omp_get_thread_num(), hostname);
 			fflush(0);
		}
	#else	
	int i;
	printf("Hello HOST\n");
	#pragma omp parallel for 
	for(i=0; i<omp_get_num_threads(); i++){
		#pragma omp critical
		printf("I am host thread %d\n", omp_get_thread_num());
	}	
	#endif
	}

	#pragma omp target if(USE_OFFLOAD) device(0)
	{

		char hostname[100];
		gethostname(hostname, 100);


		printf("I am a second target region on host %s\n", hostname);
		fflush(0);
		sleep(1);
	}
	return 0;	
}
