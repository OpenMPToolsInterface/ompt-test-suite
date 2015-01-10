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



//*****************************************************************************
// macros
//*****************************************************************************

#define SMALL_NUM_THREADS 4
#define BIG_NUM_THREADS   20



//*****************************************************************************
// global data
//*****************************************************************************

ompt_get_parallel_team_size_t my_ompt_get_parallel_team_size;



//*****************************************************************************
// interface operations 
//*****************************************************************************

void
init_test(ompt_function_lookup_t lookup) 
{
  my_ompt_get_parallel_team_size = 
    (ompt_get_parallel_team_size_t) lookup("ompt_get_parallel_team_size"); 
  
  CHECK(my_ompt_get_parallel_team_size, NOT_IMPLEMENTED,	\
	"failed to look up ompt_get_parallel_team_size");
  
  quit_on_init_failure();
}


int
regression_test(int argc, char **argv)
{
  int master_thread_id = ompt_get_thread_id();
  int nested = 0;
  
  for(nested = 0; nested <= 1; nested++) {
    omp_set_nested(nested);
    
    #pragma omp parallel num_threads(SMALL_NUM_THREADS)
    {
      int team_size_0 = omp_get_team_size(omp_get_level());
      
      CHECK(my_ompt_get_parallel_team_size(0) == team_size_0,	\
	    IMPLEMENTED_BUT_INCORRECT, "wrong team size");
      
      #pragma omp parallel num_threads(BIG_NUM_THREADS)
      {
	int team_size_1 = omp_get_team_size(omp_get_level());
	
	CHECK(my_ompt_get_parallel_team_size(0) == team_size_1,		\
	      IMPLEMENTED_BUT_INCORRECT,				\
	      "inner team size is %d but "				\
	      "ompt_get_parallel_team_size(0) reports %d",		\
	      team_size_1, my_ompt_get_parallel_team_size(0));
	
	CHECK(my_ompt_get_parallel_team_size(1) == team_size_0,		\
	      IMPLEMENTED_BUT_INCORRECT,				\
	      "outer team size is %d but "				\
	      "ompt_get_parallel_team_size(1) reports %d",		\
	      team_size_0, my_ompt_get_parallel_team_size(1));
      }
    }
    
  }
  CHECK(my_ompt_get_parallel_team_size(1000) == -1,		\
	IMPLEMENTED_BUT_INCORRECT,				\
	"need to return -1 when ancestor doesn't exist");
  
  return return_code;
}
