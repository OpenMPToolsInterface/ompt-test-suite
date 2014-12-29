#include <map>
#include <vector>
#include <iostream>
#include <string>
#include <iostream>

#include <signal.h>

#include "common.h"
#include "regex-match.h"


#define LOOKUP( lookup, fn ) fn = ( fn ## _t )lookup( #fn ); \
    if ( !fn ) \
    { \
        fprintf( stderr, "Could not lookup function %s!\n", #fn); \
	fflush(stderr); \
        assert(0); \
    }


#define macro( fn ) fn ## _t fn;
FOREACH_OMPT_FN( macro )
#undef macro
    
pthread_mutex_t thread_mutex;
pthread_mutex_t assert_mutex;
int global_error_code = CORRECT;


using namespace std;

void
warmup() 
{
    int a = omp_get_num_threads();
    #pragma omp atomic
    a += 1;
}


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



bool
register_callback(ompt_event_t e, ompt_callback_t c) {
    int code = ompt_set_callback(e, c);
    if (code != ompt_set_result_event_may_occur_callback_always) {
        return false;
    }
    return true;
}

/*
 * check whether the state sequence in vector observed_states match a regular 
 * expression specified in regex_pattern
 */ 
bool
check_states(vector<ompt_state_t>& observed_states, string regex_pattern)
{
    map<ompt_state_t, string> map; 
    #define macro( state, id) map[state] = #state;
    FOREACH_OMPT_STATE( macro )
    #undef macro
    std::string observed_states_str;
    for (int i = 0; i < observed_states.size(); i++) {
       if (map[observed_states[i]].compare("ompt_state_overhead") != 0 && 
            (i == 0 || observed_states[i] != observed_states[i-1] )) {
            observed_states_str.append(map[observed_states[i]]); 
       }
    }
    //cout << "observed_states" << endl;
    //cout << observed_states_str << endl;
    //cout << "regex_pattern" << endl;
    //cout << regex_pattern << endl;
    //cout << "regex_match_result" << endl;
    //cout << regex_match(observed_states_str, regex_pattern) << "\n" << endl;
    if (regex_match(observed_states_str, regex_pattern) == 0) {
        return true;
    }
    return false;
}

void 
print_current_states(vector<ompt_state_t>& observed_states)
{
    printf("observed_states size: %d \n", observed_states.size());
    map<ompt_state_t, string> map; 
    #define macro( state, id) map[state] = #state;
    FOREACH_OMPT_STATE( macro )
    #undef macro

    std::string observed_states_str;
    for (int i = 0; i < observed_states.size(); i++) {
        cout << map[observed_states[i]] << " \n"; 
    }
}


/*
 * try to recover from core dump
 */
void
coredump_handler(int signo)
{
    cerr << "\n****** Unexpected Segmentation Fault ******\n\n" << endl;
    exit(MIN(global_error_code, NOT_IMPLEMENTED));
}

extern "C" {
int
ompt_initialize( ompt_function_lookup_t lookup,
                 const char*            runtime_version,
                 unsigned int           ompt_version )
{
    // try to handle seg fault gracefully
    signal(SIGSEGV, coredump_handler);
    // lookup functions
    #define macro( fn ) LOOKUP( lookup, fn )
    FOREACH_OMPT_FN( macro )
    #undef macro
    pthread_mutex_init(&thread_mutex, NULL);
    pthread_mutex_init(&assert_mutex, NULL);
    init_test(lookup);
    return 1;
}
};