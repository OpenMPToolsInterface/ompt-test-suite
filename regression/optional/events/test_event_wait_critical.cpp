//*****************************************************************************
// system includes
//*****************************************************************************

#include <assert.h>
#include <iostream>
#include <map>



//*****************************************************************************
// OpenMP runtime includes
//*****************************************************************************

#include <omp.h>



//*****************************************************************************
// regression harness includes
//*****************************************************************************

#include <ompt-regression.h>
#include <ompt-initialize.h>



//*****************************************************************************
// global variables
//*****************************************************************************

volatile int waited = 0;
volatile int acquired = 0;



//*****************************************************************************
// private operations
//*****************************************************************************

static void 
on_wait_critical(ompt_wait_id_t wait_id)
{
  #pragma omp atomic update
  waited += 1;
}


static void 
on_acquired_critical(ompt_wait_id_t wait_id)
{
  #pragma omp atomic update
  acquired += 1;
}



//*****************************************************************************
// interface operations
//*****************************************************************************

void 
init_test(ompt_function_lookup_t lookup) 
{
  if (!register_callback(ompt_event_wait_critical, 
			 (ompt_callback_t) on_wait_critical)) {
    CHECK(FALSE, NOT_IMPLEMENTED, \
	  "failed to register ompt_event_wait_critical");
  }

  if (!register_callback(ompt_event_acquired_critical, 
			 (ompt_callback_t) on_acquired_critical)) {
    CHECK(FALSE, NOT_IMPLEMENTED, 
	  "failed to register ompt_event_acquired_critical");
  }

  if (return_code == NOT_IMPLEMENTED) {
    _exit(return_code);
  }
}


int
regression_test(int argc, char **argv)
{
    int nthreads; 
    #pragma omp parallel 
    {
        #pragma omp master
        {
          nthreads = omp_get_num_threads() - 1;
        }

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

    CHECK(waited == nthreads - 1, IMPLEMENTED_BUT_INCORRECT, \
	  "only %d of expected %d threads waited to enter a critical section", \
	  waited, nthreads - 1);

    CHECK(acquired == nthreads, IMPLEMENTED_BUT_INCORRECT, \
	  "only %d of %d expected threads acquired a critical section", \
	  acquired, nthreads);

    return return_code;
}
