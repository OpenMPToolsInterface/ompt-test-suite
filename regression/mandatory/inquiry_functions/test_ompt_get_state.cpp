#include <omp.h>
#include <common.h>
#include <iostream>
#include <map>
#include <vector>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <timer.h>



using namespace std;
#define NUM_THREADS 4
static bool test_implicit_barrier = false;
static bool monitor_explicit_barrier = false;
static bool test_taskwait = false;
static bool test_taskgroup = false;
vector<ompt_state_t> observed_states;

ompt_get_state_t my_ompt_get_state;

void
init_test(ompt_function_lookup_t lookup) {
    my_ompt_get_state = (ompt_get_state_t)lookup("ompt_get_state"); 
    CHECK(my_ompt_get_state, NOT_IMPLEMENTED, "ompt_get_state is not implemented");
}

pthread_mutex_t mutex_states;

bool timer_signal_blocked = false;

void 
collect_trace(ompt_state_t state)
{
    if (!timer_signal_blocked) {
        pthread_mutex_lock(&mutex_states);
        observed_states.push_back(state);
        pthread_mutex_unlock(&mutex_states);
    }
}

void
trace_collector_callback(int sig, siginfo_t *si, void *uc)
{
    ompt_wait_id_t currWait;
    ompt_state_t current_state = my_ompt_get_state(&currWait);
    observed_states.push_back(current_state);
}

int 
monitor_epilogue()
{
    timer_signal_blocked = true;
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGRTMIN);
    if (sigprocmask(SIG_SETMASK, &mask, NULL) == -1) {
        return -1;
    }
    return 0;
}

int
monitor_prelogue()
{
    timer_signal_blocked = false;
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGRTMIN);
    if (sigprocmask(SIG_UNBLOCK, &mask, NULL) == -1) {
        return -1;
    }
    pthread_mutex_lock(&mutex_states);
    observed_states.clear();
    pthread_mutex_unlock(&mutex_states);
    return 0;
}


int
main()
{
    pthread_mutex_init(&mutex_states, NULL);
    /* set up a timer */
    Timer timer; 
    init_timer(&timer);
    register_timer_callback(&timer, trace_collector_callback);
    start_timer(&timer, 100);

    /* TEST 0: Outside parallel region, expect to see ompt_state_work_serial */ 
    monitor_prelogue();
    serialwork(2);
    CHECK(check_states(observed_states, "(ompt_state_work_serial)+"),  IMPLEMENTED_BUT_INCORRECT,
           "Expect state serial outside parallel regions");
    monitor_epilogue();
 
    /* 
     * TEST1: Inside parallel region, expect to see ompt_state_work_parallel
     */
    monitor_prelogue();
    #pragma omp parallel num_threads(NUM_THREADS)
    {
        serialwork(2);
    }
    monitor_epilogue();
    CHECK(check_states(observed_states, "(ompt_state_work_parallel)+"), \ 
            IMPLEMENTED_BUT_INCORRECT, "Expect state parallel inside parallel regions");
    
    /* 
     * TEST2: In the middle of a reduction, expect to see ompt_state_work_reduction, ompt_state_work_parallel
     */
    monitor_prelogue();
    int i, accumulator = 0, N = 16;
    int master_thread_id = ompt_get_thread_id();
    #pragma omp parallel for num_threads(NUM_THREADS) private(i) reduction(+:accumulator)
    for(i=0; i < N; i++){
        if (ompt_get_thread_id() != master_thread_id) {
            serialwork(2);
        } else {
            serialwork(0);
        }
        accumulator+=(i*i);
    }
    monitor_epilogue(); 
    CHECK(check_states(observed_states, "(ompt_state_work_parallel|ompt_state_work_reduction)*(ompt_state_wait_barrier)*"), IMPLEMENTED_BUT_INCORRECT, \
                        "Expect state reduction parallel or maybe overhead in reduction regions");


    /* 
     * TEST3.1: Tesing barrier states: ompt_state_wait_barrier_explicit
     */
    monitor_prelogue();
    #pragma omp parallel num_threads(NUM_THREADS)
    {
        if (ompt_get_thread_id() != master_thread_id) {
            serialwork(2);
        }

        #pragma omp barrier

        serialwork(2);
    }
    monitor_epilogue();

    CHECK(check_states(observed_states, "((ompt_state_work_parallel)+(ompt_state_wait_barrier|ompt_state_wait_barrier_explicit)+)"), \
           IMPLEMENTED_BUT_INCORRECT, "Expect ompt_state_wait_barrier or ompt_state_wait_barrier_explicit here");

    /* 
     * TEST3.2: Tesing barrier states: ompt_state_wait_barrier_implicit
     */
    monitor_prelogue();
    #pragma omp parallel num_threads(NUM_THREADS)
    {
        if (ompt_get_thread_id() != master_thread_id) {
            serialwork(2);
        }
    }
    monitor_epilogue();
    CHECK(check_states(observed_states, "((ompt_state_work_parallel)+(ompt_state_wait_barrier|ompt_state_wait_barrier_implicit)+)"), \
           IMPLEMENTED_BUT_INCORRECT, "Expect ompt_state_wait_barrier or ompt_state_wait_barrier_implicit here");


    /*
     * TEST 4: Testing task wait states : ompt_state_wait_taskwait, ompt_state_wait_taskgroup
     */
    monitor_prelogue();
    #pragma omp parallel
    {
        #pragma omp taskgroup
        {
            int i, num_tasks = NUM_THREADS* 4;
            #pragma omp for
            for (i = 0; i < num_tasks; i++) {
                #pragma omp task 
                {
                    if (ompt_get_thread_id() != master_thread_id) {
                        serialwork(2);
                    } else {
                        serialwork(0);
                    }
                }
            }
            #pragma omp taskwait
            #pragma omp for
            for (i = 0; i < num_tasks; i++) {
                #pragma omp task 
                {
                    if (ompt_get_thread_id() != master_thread_id) {
                        serialwork(2);
                    } else {
                        serialwork(0);
                    }
                }
            }
        }
    }
    monitor_epilogue();
    //TODO differentiate ompt_state_wait_taskwait and ompt_state_wait_barrier ?
    CHECK(check_states(observed_states, \
                       "(ompt_state_wait_taskwait)+(.)*(ompt_state_wait_taskgroup)+$"), \
                       IMPLEMENTED_BUT_INCORRECT, \
                       "Expect to see ompt_state_taskwait and then ompt_state_taskgroup");

    /*
     * TEST 5.1: Testing ompt_state_wait_lock
     */
    
    omp_lock_t single_lock;
    omp_init_lock(&single_lock);
    monitor_prelogue();
    #pragma omp parallel num_threads(NUM_THREADS)
    {
        int i;
        #pragma omp for
        for (i = 0; i < NUM_THREADS * 4; i++) {
            #pragma omp task
            {
                omp_set_lock(&single_lock);
                usleep(rand() % 500000);
                serialwork(1);
                omp_unset_lock(&single_lock);
            }
        }
    }
    monitor_epilogue();
    CHECK(check_states(observed_states, \
                       "(ompt_state_work_parallel)*(ompt_state_wait_lock)+(ompt_state_work_parallel)*"), \
                       IMPLEMENTED_BUT_INCORRECT, \
                       "Expect to see a sequence of ompt_state_work_parallel and ompt_state_wait_lock");

    /*
     * TEST 5.2: Testing ompt_state_wait_nest_lock
     */
    monitor_prelogue();
    omp_nest_lock_t nest_lock;
    omp_init_nest_lock(&nest_lock);
    #pragma omp parallel num_threads(NUM_THREADS)
    {
        int i, nest_levels = 10;
        for (i = 0; i < nest_levels; i++) {
            omp_set_nest_lock(&nest_lock);
            usleep(rand() % 500000);
            omp_unset_nest_lock(&nest_lock);
        }
    }
    omp_destroy_nest_lock(&nest_lock);
    monitor_epilogue();
    //TODO: do we differentiate between wait_nest_lock and wait_lock?
    CHECK(check_states(observed_states, \
                       "(ompt_state_work_parallel)*(ompt_state_wait_nest_lock)+(ompt_state_work_parallel)*"), \
                       IMPLEMENTED_BUT_INCORRECT, \
                       "Expect to see a sequence of ompt_state_work_parallel and ompt_state_wait_nest_lock");
    

    /*
     * TEST 6: Testing ompt_state_wait_critical
     */
    monitor_prelogue();
    #pragma omp parallel num_threads(NUM_THREADS)
    {
        int i;
        #pragma omp for
        for (i = 0; i < NUM_THREADS * 4; i++) {
            if (ompt_get_thread_id() == master_thread_id) {
                usleep(10000);
            }
            #pragma omp critical
            serialwork(3);
        }
    }
    monitor_epilogue();

    //TODO: do we differentiate between critical and wait_lock?
    CHECK(check_states(observed_states, \
                       "(ompt_state_work_parallel)*(ompt_state_wait_critical)+(ompt_state_work_parallel)*"), \
                       IMPLEMENTED_BUT_INCORRECT, \
                       "Expect to see a sequence of ompt_state_work_parallel and wait_critical");
        
    /*
     * TEST 7: Testing ompt_state_wait_atomic
     */
    monitor_prelogue();
    long atomic_number = 0;
    #pragma omp parallel num_threads(NUM_THREADS)
    {
        int i;
        #pragma omp for
        for (i = 0; i < NUM_THREADS * 10; i++) {
            if (ompt_get_thread_id() == master_thread_id) {
                usleep(10000);
            }
            #pragma omp atomic update
            atomic_number += 1;
        }
    }
    monitor_epilogue();
    CHECK(check_states(observed_states, \
                       "(ompt_state_wait_atomic)+"), \
                       IMPLEMENTED_BUT_INCORRECT, \
                       "Expect to see ompt_state_wait_atomic");
    /*
     * TEST 8: Testing ompt_state_wait_ordered
     */
    monitor_prelogue();
    vector<int> sequence; // can use this to check the correctness of the ordered construct
    #pragma omp parallel for private(i) ordered schedule(dynamic)
    for (i=0; i<1000; i++) 
    {
        if (ompt_get_thread_id() == master_thread_id) {
            usleep(1000);
        }
        #pragma omp ordered
        {
            sequence.push_back(i);
        }
    }
    monitor_epilogue();
    //TODO: do we differentiate between ompt_state_wait_barrier and ordered
    CHECK(check_states(observed_states, \
                       "(ompt_state_wait_ordered)+"), \
                       IMPLEMENTED_BUT_INCORRECT, \
                       "Expect to see ompt_state_wait_ordered");
     
    return 0;

}
