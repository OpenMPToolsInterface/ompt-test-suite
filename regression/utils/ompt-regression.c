//*****************************************************************************
// system include files 
//*****************************************************************************

#include <stdio.h>
#include <signal.h>



//*****************************************************************************
// local include files 
//*****************************************************************************

#include "ompt-regression.h"
#include "ompt-openmp.h"



//*****************************************************************************
// global variables
//*****************************************************************************
    
pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t assert_mutex = PTHREAD_MUTEX_INITIALIZER; 

int return_code = CORRECT;
const char *regression_test_name = "";



//*****************************************************************************
// private functions 
//*****************************************************************************

static void
segv_handler(int signo)
{
  static int reported_segv = 0;
  pthread_mutex_lock(&assert_mutex);
  if (reported_segv++ == 0) {
    printf("  %s: error -- failed with segmentation fault\n", 
	   regression_test_name);
  }
  pthread_mutex_unlock(&assert_mutex);
  exit(MIN(return_code, FATAL));
}



//*****************************************************************************
// interface functions 
//*****************************************************************************

void
serialwork(int workload)
{
    int i = 0;
    for (i = 0; i < workload; i++) {
        usleep(500000);
    }
    for (i = 0; i < 10000; i++) {
        void * p = (void *) malloc(10);
        free(p);
    }
}


int
main(int argc, char **argv)
{
  // save the executable name for error messages
  regression_test_name = argv[0];

  // intialize a signal handler to catch segmentation faults so tests can 
  // fail more gracefully if unavailable functions are called
  signal(SIGSEGV, segv_handler);

  // force initialization of the openmp runtime system
  openmp_init();

  // execute the user regression test
  return regression_test(argc, argv);
}
