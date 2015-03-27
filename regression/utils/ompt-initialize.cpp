//*****************************************************************************
// system include files 
//*****************************************************************************

#include <stdio.h>
#include <signal.h>

#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>



//*****************************************************************************
// OpenMP runtime includes
//*****************************************************************************
#include <omp.h>



//*****************************************************************************
// local include files 
//*****************************************************************************

#include "ompt-initialize.h"
#include "ompt-regression.h"



//*****************************************************************************
// macros
//*****************************************************************************

#define DEFINE_OMPT_FN_PTR( lookup, fn ) \
    fn = ( fn ## _t )lookup( #fn ); \
    if ( !fn ) \
    { \
        fprintf( stderr, "Could not lookup function %s!\n", #fn); \
	fflush(stderr); \
        assert(0); \
    }



//*****************************************************************************
// global variables
//*****************************************************************************
    
#define macro( fn ) fn ## _t fn;
FOREACH_OMPT_INQUIRY_FN( macro )
#undef macro



//*****************************************************************************
// interface functions 
//*****************************************************************************


void
quit_on_init_failure()
{
  if (return_code == NOT_IMPLEMENTED) exit(return_code);
}


int
register_callback(ompt_event_t e, ompt_callback_t c) 
{
  int code = ompt_set_callback(e, c);
  if (code != ompt_set_result_event_may_occur_callback_always) {
    return FALSE;
  }
  return TRUE;
}


extern "C" {

// this must have a C-style interface because that is what an 
// OpenMP runtime expects
int
ompt_initialize( ompt_function_lookup_t lookup,
                 const char*            runtime_version,
                 unsigned int           ompt_version )
{
  // initialize OMPT function pointers
#define macro( fn ) DEFINE_OMPT_FN_PTR( lookup, fn )
  FOREACH_OMPT_INQUIRY_FN( macro )
#undef macro

  init_test(lookup);

  if (return_code == FATAL || 
      return_code == NOT_IMPLEMENTED) {
    _exit(return_code);
  }

  return 1;
}

};
