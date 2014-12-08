#include <omp.h>
#include <iostream>
#include <map>
#include <assert.h>
#include <common.h>

using namespace std;
long thread_base_index = 0;
map<long, OMPTAutomatonState *> threadId_to_state;
pthread_mutex_t map_mutex;

void on_wait_critical(ompt_wait_id_t wait_id)
{
    long thread_index = fetch_and_increment(&thread_base_index);
    OMPTAutomatonState state;
    // ompt_state_t allowed_states [] = {ompt_state_wait_critical};
    // check_state(allowed_states, 1);
    state.last_event = ompt_event_wait_critical;
    
    pthread_mutex_lock(&map_mutex);
    threadId_to_state[thread_index] = &state;
    pthread_mutex_unlock(&map_mutex);
}

void on_acquired_critical(ompt_wait_id_t wait_id)
{
    long thread_index = fetch_and_increment(&thread_base_index);
    OMPTAutomatonState * state = threadId_to_state[thread_index];
    assert(state);
    assert(state->last_event == ompt_event_wait_critical);
    
    // number of critical section entries should be odd
    assert(state->num_critical_entries % 2 == 1);
    state->last_event = ompt_event_acquired_critical;
}

void init_test(ompt_function_lookup_t lookup) {
    pthread_mutex_init(&map_mutex, NULL);
    REG_CB(wait_critical);
    REG_CB(acquired_critical);
}

void critical_section() {
    int a = 1;
    a++;
}

int main()
{
    printf("\n>>> Test Critical: start\n");
    
    #pragma omp parallel num_threads(NUM_THREADS)
    {
        #pragma omp critical
        {
            critical_section();
        }
    }
    
    for (long i = 0; i < NUM_THREADS; i++) {
        OMPTAutomatonState * state = threadId_to_state[i];
        assert(state);
        assert(state->last_event == ompt_event_acquired_critical);
        assert(state->num_critical_entries % 2 == 0);
    }
    
    printf("  Test Critical: success\n");
}
