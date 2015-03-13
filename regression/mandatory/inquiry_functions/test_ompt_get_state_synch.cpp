//*****************************************************************************
// system includes
//*****************************************************************************

#include <map>
#include <vector>

#include <signal.h>



//*****************************************************************************
// OpenMP runtime includes
//*****************************************************************************

#include <omp.h>



//*****************************************************************************
// regression harness includes
//*****************************************************************************

#include <ompt-regression.h>
#include <ompt-initialize.h>

#include <states.h>
#include <timer.h>



//*****************************************************************************
// macros
//*****************************************************************************

#define NUM_THREADS 4
#define VECTOR_LENGTH 20000000



//*****************************************************************************
// global data
//*****************************************************************************

ompt_get_state_t ompt_get_state_fn;



//*****************************************************************************
// local operations
//*****************************************************************************

static const char *
state_name(ompt_state_t s)
{
#define name_from_state(name, code) case code: return #name ; 

  switch(s) {
  FOREACH_OMPT_STATE(name_from_state);
  default: return "illegal ompt_state_t code";
  }
#undef name_from_state
}


static void
synchronous_state_test()
{
  ompt_wait_id_t wait_id;
  ompt_state_t state = ompt_get_state_fn(&wait_id);
  CHECK(state == ompt_state_work_serial, IMPLEMENTED_BUT_INCORRECT, 
        "before a parallel region: expected ompt_state_work_serial, got %s", state_name(state)); 

  #pragma omp parallel
  {
    ompt_wait_id_t l1_wait_id;
    ompt_state_t l1_state = ompt_get_state_fn(&l1_wait_id);
    CHECK(l1_state == ompt_state_work_parallel, IMPLEMENTED_BUT_INCORRECT, 
          "in parallel region: expected ompt_state_work_parallel, got %s", state_name(state)); 

    #pragma omp parallel
    {
      ompt_wait_id_t l2_wait_id;
      ompt_state_t l2_state = ompt_get_state_fn(&l2_wait_id);
      CHECK(l2_state == ompt_state_work_parallel, IMPLEMENTED_BUT_INCORRECT, 
          "in parallel region: expected ompt_state_work_parallel, got %s", state_name(state)); 
    }

    l1_state = ompt_get_state_fn(&l1_wait_id);
    CHECK(l1_state == ompt_state_work_parallel, IMPLEMENTED_BUT_INCORRECT, 
          "in parallel region: expected ompt_state_work_parallel, got %s", state_name(state)); 
  }

  state = ompt_get_state_fn(&wait_id);
  CHECK(state == ompt_state_work_serial, IMPLEMENTED_BUT_INCORRECT, 
        "after a parallel region: expected ompt_state_work_serial, got %s", state_name(state)); 
}



//*****************************************************************************
// interface operations
//*****************************************************************************

void
init_test(ompt_function_lookup_t lookup) 
{
  ompt_get_state_fn = (ompt_get_state_t)lookup("ompt_get_state"); 
  CHECK(ompt_get_state_fn, NOT_IMPLEMENTED,	\
	"failed to register ompt_get_state");
  quit_on_init_failure();
}


int
regression_test(int argc, char **argv)
{
  synchronous_state_test();

  return 0;
}
