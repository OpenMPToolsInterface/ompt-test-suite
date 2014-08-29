#include <iostream>
#include <assert.h>

#include <omp.h>
#include <ompt.h>


/*******************************************************************
 * global variables
 *******************************************************************/

int WAIT_FOR_TASK_ID = 0;

#define OMPT_FN_TYPE(fn) fn ## _t 
#define OMPT_FN_LOOKUP(lookup,fn) fn = (OMPT_FN_TYPE(fn)) lookup(#fn)
#define OMPT_FN_DECL(fn) OMPT_FN_TYPE(fn) fn

OMPT_FN_DECL(ompt_get_task_id);


int ompt_initialize(ompt_function_lookup_t lookup, const char *runtime_version, 
		    int ompt_version) 
{

  /* look up and bind OMPT API functions */
  OMPT_FN_LOOKUP(lookup,ompt_get_task_id);
  return 1;
}

void check_task_id( ompt_task_id_t current, ompt_task_id_t parent1, ompt_task_id_t parent2,
		    const char *info )
{
  if (parent1 != parent2) {
  #pragma omp critical
  {
	std::cout <<"TEST FAILED: parent task IDs are inconsistent  " << std::endl
		  << "inside parallel region: "<< info << std::endl 
		  << "\tcurrent task=" << current << std::endl
		  << "\tparent1 task=" << parent1 << std::endl
		  << "\tparent2_task=" << parent2 << std::endl;
      }
      assert(parent1 == parent2);
  }
}

void test_parallel()
{
  // capture the serial task id
  ompt_task_id_t serial_task_id = ompt_get_task_id(0); 
  assert(serial_task_id != 0);

  #pragma omp parallel num_threads(1) 
  {
    ompt_task_id_t implicit_task_id = ompt_get_task_id(0);
    ompt_task_id_t parent_task_id = ompt_get_task_id(1);
    
    check_task_id(implicit_task_id, parent_task_id, serial_task_id, "outer parallel region");

     // check the correctness of task id for the nested loop 
    #pragma omp parallel num_threads(1)
    {
    	ompt_task_id_t nested_task_id = ompt_get_task_id(0);
    	ompt_task_id_t parent_nested_task_id = ompt_get_task_id(1);

    	check_task_id( 	nested_task_id, parent_nested_task_id, 
			implicit_task_id, "inner nested region");
    }
  }
}


// test whether nesting of tasks is right for a simple parallel region with 1 thread
int 
main(int argc, char *argv[])
{
  ompt_task_id_t taskid = ompt_get_task_id(0);
  assert(taskid == 0);

  // force OpenMP initialization
  omp_get_max_threads();

  test_parallel();

  return 0;
}
