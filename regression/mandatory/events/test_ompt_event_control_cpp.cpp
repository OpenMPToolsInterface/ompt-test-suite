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
// private operations
//*****************************************************************************

static void 
on_ompt_event_control(uint64_t command,  uint64_t modifier)
{
  if(command == 101 && modifier == 212) {
    exit(CORRECT);
  }
}



//*****************************************************************************
// interface operations
//*****************************************************************************

void 
init_test(ompt_function_lookup_t lookup)
{
  if (!register_callback(ompt_event_control, 
			 (ompt_callback_t) on_ompt_event_control)) {
    CHECK(false, FATAL, "failed to register ompt_event_control");
  }
}


int
regression_test(int argc, char** argv)
{
  ompt_control(101, 212);
  return IMPLEMENTED_BUT_INCORRECT;
}
