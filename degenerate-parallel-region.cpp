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

void test_parallel()
{
  // capture the serial task id
  ompt_task_id_t serial_task_id = ompt_get_task_id(0); 
  assert(serial_task_id != 0);

  #pragma omp parallel num_threads(1) 
  {
    ompt_task_id_t implicit_task_id = ompt_get_task_id(0);
    ompt_task_id_t parent_task_id = ompt_get_task_id(1);
    
    if (parent_task_id != serial_task_id) {
      #pragma omp critical
      {
	std::cout <<"TEST FAILED: parent task should be the same as serial task" << std::endl
		  << "inside parallel region: "<< std::endl 
		  << "\timplicit_task=" << implicit_task_id << std::endl
		  << "\tparent_task=" << parent_task_id << std::endl
		  << "\tserial_task=" << serial_task_id << std::endl;
      }
      assert(parent_task_id == serial_task_id);
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
