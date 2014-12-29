#ifndef common_h
#define	common_h

#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <omp.h>
#include <ompt.h>

#define TRUE  1
#define FALSE 0

extern pthread_mutex_t thread_mutex;
extern pthread_mutex_t assert_mutex;
extern int global_error_code;

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

// Macro to set an OMPT callback (input event name e.g. "loop_begin", will set "on_loop_begin" as callback function)
#define REG_CB( EVENT ) \
    if ( ompt_set_callback( ompt_event_##EVENT, ( ompt_callback_t )on_##EVENT ) != ompt_set_result_event_may_occur_callback_always ) \
    { \
        fprintf(stderr, "\tFailed to register OMPT callback %s!\n", #EVENT ); \
	fflush(stderr); \
        exit(NOT_IMPLEMENTED); \
    }

#define CHECK(__condition, __error_code, __message...) if (!(__condition)) { \
    pthread_mutex_lock(&assert_mutex); \
    printf("\tERROR in Test[%s] Line[%d]: \n",  __FILE__, __LINE__); printf("\t\t" __message "\n" ); \
    pthread_mutex_unlock(&assert_mutex); \
    global_error_code = MIN(global_error_code, __error_code); }

/* Declaration of ompt functions */
#define macro( fn ) extern fn ## _t fn;
FOREACH_OMPT_FN( macro )
#undef macro

#define NOT_IMPLEMENTED -2
#define IMPLEMENTED_BUT_INCORRECT -1 

#define CORRECT 0
#define CONTINUE 200


#if defined(__cplusplus)
extern "C" {
#endif

void register_segv_handler(char **argv);

void warmup();
void serialwork(int workload);
int register_callback(ompt_event_t e, ompt_callback_t c);
// Callback function called after OMPT was initialized. E.g. for setting event callbacks
extern void init_test(ompt_function_lookup_t lookup);

#if defined(__cplusplus)
};
#endif

#endif	/* common_h */

