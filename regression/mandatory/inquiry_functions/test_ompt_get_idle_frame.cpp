//*****************************************************************************
// system includes
//*****************************************************************************

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
// macros
//*****************************************************************************

#define NUM_THREADS 4



//*****************************************************************************
// global data
//*****************************************************************************

ompt_get_idle_frame_t my_ompt_get_idle_frame;



//*****************************************************************************
// interface operations
//*****************************************************************************

void
init_test(ompt_function_lookup_t lookup) 
{
  my_ompt_get_idle_frame = 
    (ompt_get_idle_frame_t) lookup("ompt_get_idle_frame"); 

  CHECK(my_ompt_get_idle_frame, NOT_IMPLEMENTED, 
	"failed to register ompt_get_idle_frame");

  quit_on_init_failure();
}


int
regression_test(int argc, char **argv)
{
  int master_thread_id = ompt_get_thread_id();
  
  // enable nested parallelism
  omp_set_nested(1);
  
  CHECK(my_ompt_get_idle_frame() == NULL, IMPLEMENTED_BUT_INCORRECT,	\
	"initial thread should always have a null idle frame");
  
  #pragma omp parallel num_threads(NUM_THREADS)
  {
    if (ompt_get_thread_id() == master_thread_id) {
      CHECK(my_ompt_get_idle_frame() == NULL, IMPLEMENTED_BUT_INCORRECT, \
	    "initial thread should always have a null idle frame");
    }
    
    #pragma omp parallel num_threads(NUM_THREADS)
    {
      serialwork(1);
      if (ompt_get_thread_id() == master_thread_id) {
	CHECK(my_ompt_get_idle_frame() == NULL, IMPLEMENTED_BUT_INCORRECT, \
	      "initial thread should always have a null idle frame");
      }
    }
  }
  
  // If nested parallelism is disabled or not supported,
  // then the new team that is created by a thread encountering a 
  // parallel construct inside a parallel region will consist only 
  // of the encountering thread. Thus, in this case, we should be able 
  // to see a consistent idle frame for each thread.

  omp_set_nested(0);

  std:: map<int, void *> threadId_to_idleFrame;

  #pragma omp parallel num_threads(NUM_THREADS)
  {
    #pragma omp critical
    {
      threadId_to_idleFrame[ompt_get_thread_id()] = my_ompt_get_idle_frame();
    }
    
    #pragma omp parallel num_threads(NUM_THREADS)
    {
      #pragma omp critical
      {
	CHECK(threadId_to_idleFrame[ompt_get_thread_id()] == my_ompt_get_idle_frame(), \
	      IMPLEMENTED_BUT_INCORRECT, "thread should have a consistent idle frame");
      }
      #pragma omp parallel num_threads(NUM_THREADS)
      {
        #pragma omp critical
	{
	  CHECK(threadId_to_idleFrame[ompt_get_thread_id()] == my_ompt_get_idle_frame(), \
		IMPLEMENTED_BUT_INCORRECT, "thread should have a consistent idle frame");
	}
      }
    }
  }
  
  CHECK(my_ompt_get_idle_frame() == NULL, IMPLEMENTED_BUT_INCORRECT,	\
	"initial thread should always have a null idle frame");
  
  return return_code;
}

