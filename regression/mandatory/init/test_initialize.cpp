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
// global variables
//*****************************************************************************

int init_success = 0;



//*****************************************************************************
// interface operations
//*****************************************************************************

extern "C" {
#ifdef OMPT_V2013_07

int ompt_initialize(void)
{
#if DEBUG
  printf("ompt_initialize(void) called\n");
#endif
  init_success = 1;
  return 1;
}


#else

int ompt_initialize(ompt_function_lookup_t lookup, 
                    const char *runtime_version, 
                    unsigned int ompt_version)
{
#if DEBUG
  printf("ompt_initialize(lookup = %p, runtime_version = %s, "
         "ompt_version = %d)\n", runtime_version, ompt_version);
#endif
  init_success = 1;
  return 1;
}


#endif
};


int
regression_test(int argc, char **argv)
{
  int max_threads = omp_get_max_threads();

  CHECK((max_threads > 0), IMPLEMENTED_BUT_INCORRECT, \
	"ompt_get_max_threads() returned %d <= 0\n", max_threads);

  CHECK(init_success, NOT_IMPLEMENTED, "ompt_initialize was not called\n");

  return return_code;
}
