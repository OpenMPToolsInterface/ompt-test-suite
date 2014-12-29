//*****************************************************************************
// system include files 
//*****************************************************************************

#include <stdio.h>
#include <signal.h>



//*****************************************************************************
// local include files 
//*****************************************************************************

#include "error.h"



//*****************************************************************************
// global variables
//*****************************************************************************
    
pthread_mutex_t thread_mutex;
pthread_mutex_t assert_mutex;

int global_error_code = CORRECT;
const char *executable_name = "";



//*****************************************************************************
// private functions 
//*****************************************************************************

static void
segv_handler(int signo)
{
    printf("  %s: error -- failed with segmentation fault\n", executable_name);
    exit(MIN(global_error_code, NOT_IMPLEMENTED));
}



//*****************************************************************************
// interface functions 
//*****************************************************************************

void register_segv_handler(char **argv)
{
  executable_name = argv[0];
  signal(SIGSEGV, segv_handler);
}
