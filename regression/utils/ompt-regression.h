#ifndef ompt_regression_h
#define	ompt_regression_h

//*****************************************************************************
// system includes
//*****************************************************************************

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>



//*****************************************************************************
// macros
//*****************************************************************************

#define TRUE  1
#define FALSE 0


#define MIN(x, y) (((x) < (y)) ? (x) : (y))


#define CHECK(__condition, __error_code, ...) \
  if (!(__condition)) {			      \
    pthread_mutex_lock(&assert_mutex); \
    printf("  %s: error at %s:%d",  regression_test_name, \
           __FILE__, __LINE__); printf(" -- "  __VA_ARGS__ ); \
    printf("\n"); \
    fflush(NULL); \
    return_code = MIN(return_code, __error_code); \
    pthread_mutex_unlock(&assert_mutex); \
  }


//-----------------------------------------------------------------------------
// regression test return codes
//-----------------------------------------------------------------------------

#define FATAL -4
#define OMPT_SHUTDOWN_FAILED_TO_PREEMPT_EXIT -3
#define NOT_IMPLEMENTED -2
#define IMPLEMENTED_BUT_INCORRECT -1 
#define CORRECT 0

#if defined(__cplusplus)
extern "C" {
#endif

//*****************************************************************************
// global variables
//*****************************************************************************
extern pthread_mutex_t thread_mutex;
extern pthread_mutex_t assert_mutex;

extern const char *regression_test_name;
extern int return_code;
extern int ompt_initialized;



//*****************************************************************************
// interface functions
//*****************************************************************************

extern int regression_test(int argc, char **argv);

extern void serialwork(int workload);


#if defined(__cplusplus)
};
#endif


#endif

