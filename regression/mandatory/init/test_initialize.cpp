//*****************************************************************************
// system includes
//*****************************************************************************

#include <stdio.h>



//*****************************************************************************
// OpenMP runtime includes
//*****************************************************************************

#include <omp.h>
#include <ompt.h>



//*****************************************************************************
// regression harness includes
//*****************************************************************************

#include <ompt-regression.h>



//*****************************************************************************
// macros
//*****************************************************************************

#define DEBUG 0



//*****************************************************************************
// interface operations
//*****************************************************************************

extern "C" {

void 
ompt_initialize(ompt_function_lookup_t lookup, 
                const char *runtime_version, 
                unsigned int ompt_version)
{
#if DEBUG
  printf("ompt_initialize(lookup = %p, runtime_version = %s, "
         "ompt_version = %d)\n", lookup, runtime_version, ompt_version);
#endif
  ompt_initialized = 1;
}


ompt_initialize_t 
ompt_tool(void)
{
  return ompt_initialize;
}


};


int
regression_test(int argc, char **argv)
{
  // harness will guarantee that ompt_initialized has been set.

  return return_code;
}
