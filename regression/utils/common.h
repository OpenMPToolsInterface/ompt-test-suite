#ifndef COMMON_H
#define	COMMON_H

#include <omp.h>
#include <ompt.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <vector>
#include <string>

using namespace std;
extern pthread_mutex_t thread_mutex;
extern pthread_mutex_t assert_mutex;
extern int global_error_code;

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

// Macro to set an OMPT callback (input event name e.g. "loop_begin", will set "on_loop_begin" as callback function)
#define REG_CB( EVENT ) \
    if ( ompt_set_callback( ompt_event_##EVENT, ( ompt_callback_t )on_##EVENT ) != ompt_set_result_event_may_occur_callback_always ) \
    { \
        fprintf(stderr, "Failed to register OMPT callback %s!\n", #EVENT ); \
	fflush(stderr); \
        exit(NOT_IMPLEMENTED); \
    }

#define CHECK(__condition, __error_code, __message...) if (!(__condition)) { \
    pthread_mutex_lock(&assert_mutex); \
    printf("\n\nERROR in Test[%s] Line[%d]: \n",  __FILE__, __LINE__); printf(__message); printf("\n\n");   \
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

void warmup();
void print_current_states(vector<ompt_state_t>& observed_states);
bool check_states(vector<ompt_state_t>& observed_states, string regex_pattern);
void check_current_state(ompt_state_t expected_state);
void serialwork(int workload);
bool register_callback(ompt_event_t e, ompt_callback_t c);
// Callback function called after OMPT was initialized. E.g. for setting event callbacks
extern void init_test(ompt_function_lookup_t lookup);

#endif	/* COMMON_H */

