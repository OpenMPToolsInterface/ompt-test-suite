#include <omp.h>
#include <iostream>
#include <map>
#include <assert.h>
#include <common.h>

using namespace std;
pthread_mutex_t mutex;

volatile int waited = 0;
volatile int acquired = 0;

void on_wait_critical(ompt_wait_id_t wait_id)
{
  #pragma omp atomic update
  waited += 1;
}

void on_acquired_critical(ompt_wait_id_t wait_id)
{
  #pragma omp atomic update
  acquired += 1;
}

void init_test(ompt_function_lookup_t lookup) 
{
    if (!register_callback(ompt_event_wait_critical, (ompt_callback_t) on_wait_critical)) {
        CHECK(FALSE, FATAL, "failed to register ompt_event_wait_critical");
    }

    if (!register_callback(ompt_event_acquired_critical, (ompt_callback_t) on_acquired_critical)) {
        CHECK(FALSE, FATAL, "failed to register ompt_event_acquired_critical");
    }
}

int main(int argc, char **argv)
{
    register_segv_handler(argv);

    int max_threads = omp_get_max_threads();
    int expected_waiters = max_threads - 1;

    #pragma omp parallel num_threads(max_threads)
    {
	int myid = omp_get_thread_num();

	// delay everyone but the master
	if (myid != 0) {
	   sleep(2);
	}

        #pragma omp critical
        {
            #pragma omp master
            {
              // master waits longer then everyone else, so they try to enter 
              // the critical section while the master still is in it.
              // that should cause all other workers to receive event 
              // ompt_event_wait_critical. 
              sleep(10);
            }
        }
    }

    CHECK(waited == expected_waiters, IMPLEMENTED_BUT_INCORRECT, "only %d of expected %d threads waited to enter a critical section", waited, expected_waiters);
    CHECK(acquired == max_threads, IMPLEMENTED_BUT_INCORRECT, "only %d of %d expected threads acquired a critical section", acquired, max_threads);
}
