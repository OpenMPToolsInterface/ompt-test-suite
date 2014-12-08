#ifndef COMMON_H
#define	COMMON_H

#include <ompt.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <vector>
#include <string>

using namespace std;
// Macro to set an OMPT callback (input event name e.g. "loop_begin", will set "on_loop_begin" as callback function)
#define REG_CB( EVENT ) \
    if ( ompt_set_callback( ompt_event_##EVENT, ( ompt_callback_t )on_##EVENT ) != ompt_set_result_event_may_occur_callback_always ) \
    { \
        fprintf(stderr, "Failed to register OMPT callback %s!\n", #EVENT ); \
	fflush(stderr); \
        exit(NOT_IMPLEMENTED); \
    }

extern pthread_mutex_t thread_mutex;
extern pthread_mutex_t assert_mutex;
#define ASSERT(_x, _code, _args...) if (!(_x)) { \
    pthread_mutex_lock(&assert_mutex); \
    printf("\n\nERROR: "); printf(_args); printf("\n\n");   \
    pthread_mutex_unlock(&assert_mutex); \
    if (((_code) != CONTINUE)) { \
    exit(_code); }}
/* Declaration of ompt functions */
#define macro( fn ) extern fn ## _t fn;
FOREACH_OMPT_FN( macro )
#undef macro

#define NOT_IMPLEMENTED -2
#define IMPLEMENTED_BUT_INCORRECT -1 
#define IMPLEMENTED_INCORRECT -1

#define CORRECT 0
#define CONTINUE 200

void print_current_states(vector<ompt_state_t>& observed_states);
bool check_states(vector<ompt_state_t>& observed_states, string regex_pattern);
class OMPTAutomatonState
{
    public:
        ompt_event_t last_event;
        int num_critical_entries;
        
    private:
};
void check_current_state(ompt_state_t expected_state);
long fetch_and_increment(long * number);
// Callback function called after OMPT was initialized. E.g. for setting event callbacks
extern void init_test(ompt_function_lookup_t lookup);

#endif	/* COMMON_H */

