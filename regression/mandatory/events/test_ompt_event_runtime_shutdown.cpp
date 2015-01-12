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
// macros
//*****************************************************************************

#define NUM_THREADS 4



//*****************************************************************************
// private operations 
//*****************************************************************************

void 
on_ompt_event_runtime_shutdown(uint64_t command,  uint64_t modifier)
{
  _exit(return_code);
}



//*****************************************************************************
// interface operations 
//*****************************************************************************

void 
init_test(ompt_function_lookup_t lookup)
{
  if (!register_callback(ompt_event_runtime_shutdown, 
			 (ompt_callback_t) on_ompt_event_runtime_shutdown)) {
    CHECK(FALSE, FATAL, "failed to register ompt_event_runtime_shutdown");
  }
}


int
regression_test(int argc, char** argv)
{
  #pragma omp parallel num_threads(NUM_THREADS)
  {
    #pragma omp parallel num_threads(NUM_THREADS) 
    {
      serialwork(1);
    }
  }
  return OMPT_SHUTDOWN_FAILED_TO_PREEMPT_EXIT;
}
