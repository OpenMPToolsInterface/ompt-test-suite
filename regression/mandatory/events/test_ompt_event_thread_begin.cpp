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

#define DEBUG 0



//*****************************************************************************
// global data
//*****************************************************************************

volatile int nthreads_expected = 0;
volatile int nthreads_created = 0;

std::map<ompt_task_id_t, ompt_thread_type_t> thread_id_map;



//*****************************************************************************
// private operations
//*****************************************************************************

static void
on_ompt_event_thread_begin(ompt_thread_type_t thread_type, 
			   ompt_thread_id_t thread_id)
{
  static int first = 1;
  
  pthread_mutex_lock(&thread_mutex);
  nthreads_created += 1;
  
#if DEBUG
  printf("thread_id = %d thread_type = %d\n", thread_id, thread_type);
  fflush(NULL);
#endif
  
  if (first == 1) {
    CHECK(thread_type == ompt_thread_initial, IMPLEMENTED_BUT_INCORRECT, \
	  "expect to see ompt_thread_initial (%d) but got %d",		\
	  ompt_thread_initial, thread_type);

    first = 0;
  } else {
    CHECK(thread_type == ompt_thread_worker ||				\
	  thread_type == ompt_thread_other, IMPLEMENTED_BUT_INCORRECT,	\
	  "expected to see ompt_thread_worker or "			\
	  "ompt_thread_other after initial thread");

    CHECK(thread_id_map.count(thread_id) == 0, \
	  IMPLEMENTED_BUT_INCORRECT, "duplicate thread ids");
  }
  thread_id_map[thread_id] = thread_type;
  pthread_mutex_unlock(&thread_mutex);
}



//*****************************************************************************
// interface operations
//*****************************************************************************

void 
init_test(ompt_function_lookup_t lookup)
{
    if (!register_callback(ompt_event_thread_begin, 
			   (ompt_callback_t) on_ompt_event_thread_begin)) {
        CHECK(false, FATAL, "failed to register ompt_event_thread_begin");
    }
}


int
regression_test(int argc, char** argv)
{
  #pragma omp parallel 
  {
    #pragma omp atomic update
    nthreads_expected += 1;
  }
  
  
#if DEBUG
  printf("num threads created = %d\n", nthreads_created);
#endif
  
  CHECK(nthreads_created == nthreads_expected, IMPLEMENTED_BUT_INCORRECT, \
	"wrong number of calls to ompt_event_thread_begin");
  
  return return_code;
}
