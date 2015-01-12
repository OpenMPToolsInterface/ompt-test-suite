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

#define NUM_THREADS 1


//*****************************************************************************
// global data
//*****************************************************************************

std::map<ompt_task_id_t, ompt_task_id_t> task_id_to_task_id_map;

ompt_get_task_id_t my_ompt_get_task_id;



//*****************************************************************************
// interface operations
//*****************************************************************************

void 
init_test(ompt_function_lookup_t lookup)
{
  my_ompt_get_task_id = 
    (ompt_get_task_id_t)lookup("ompt_get_task_id");
  
  CHECK(my_ompt_get_task_id, NOT_IMPLEMENTED,  
	"failed to register my_ompt_get_task_id");
  
  quit_on_init_failure();
}


int
regression_test(int argc, char** argv)
{
  #pragma omp parallel num_threads(1)
  {
    #pragma omp master
    {
      ompt_task_id_t level0_task_id = my_ompt_get_task_id(0);
      #pragma omp task
      {
	ompt_task_id_t  level1_task_id = my_ompt_get_task_id(0);

	CHECK(level0_task_id == my_ompt_get_task_id(1),		\
	      IMPLEMENTED_BUT_INCORRECT,			\
	      "level 1 parent task id is inconsistent");

        #pragma omp task
	{
	  ompt_task_id_t  level2_task_id = my_ompt_get_task_id(0);

	  CHECK(level1_task_id == my_ompt_get_task_id(1),	\
		IMPLEMENTED_BUT_INCORRECT,			\
		"level 2 parent task id is inconsistent");

	  CHECK(level0_task_id == my_ompt_get_task_id(2),	\
		IMPLEMENTED_BUT_INCORRECT,			\
		"level 2 grandparent task id is inconsistent");

          #pragma omp task
	  {
	    CHECK(level2_task_id == my_ompt_get_task_id(1),	\
		  IMPLEMENTED_BUT_INCORRECT,			\
		  "level 3 parent task id is inconsistent");
	    CHECK(level1_task_id == my_ompt_get_task_id(2),	\
		  IMPLEMENTED_BUT_INCORRECT,			\
		  "level 3 parent task id is inconsistent");
	  }
	}
      }
    }
  }

  CHECK(my_ompt_get_task_id(1000) == 0, \
	IMPLEMENTED_BUT_INCORRECT,	\
	"test at an invalid depth");

  return return_code;
}
