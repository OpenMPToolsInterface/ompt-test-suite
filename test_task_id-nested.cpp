#include <map>
#include <vector>
#include <iostream>
#include <assert.h>

#include <omp.h>
#include <ompt.h>


/*******************************************************************
 * global variables
 *******************************************************************/


// map of all task id and the parent
typedef std::map<ompt_task_id_t, ompt_task_id_t> MapTaskID;

// list of parent task IDs
static MapTaskID map_taskID, map_siblings;


/*******************************************************************
 * ompt initialization
 *******************************************************************/

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


/*******************************************************************
 * helper functions
 *******************************************************************/

static bool
is_task_unique(MapTaskID &task_list, ompt_task_id_t id, ompt_task_id_t parent)
{
  if (task_list.size()>0)
  {
    MapTaskID::const_iterator it = task_list.find(id);
    // id must be unique
    if ( it != task_list.end() ) {
	std::cout<<" ID already exist: " << id << " (parent: " << parent
		 << ") vs. " << it->first << " (parent: " << it->second << ")" << std::endl;
	assert( it == task_list.end() );
    }
  }
  {
    task_list[id] = parent;
  }
  return true;
}


/*******************************************************************
 * this should test if two tasks have the same parent
 * however, it just checks if there's already the same parent ID 
 * or not.
 *******************************************************************/
static bool
has_the_same_parent(MapTaskID &task_list, ompt_task_id_t parent)
{
  if (task_list.size()>0) {
    MapTaskID::const_iterator it = task_list.find(parent);
    // if the task has a parent, check if the parent is the same
    if ( it != task_list.end()) {
       if  (it->second != parent) {
  	 {
	  std::cout << "id " <<  ompt_get_task_id(0) << ": don't have the same parent: " 
		    << it->second << " vs. " << parent  << std::endl;
	  assert(it->second == parent);
	  }
       }
       // the task has the same parent with its sibling: returns true
       return true;
    }
  }
  // add the parent to the list
  {
    task_list[parent] = parent;
  }
  return true;
}


/*******************************************************************
 * main test
 *******************************************************************/

void test_parallel()
{
  // capture the serial task id
  ompt_task_id_t serial_task_id = ompt_get_task_id(0); 
  assert(serial_task_id != 0);

  // testing omp parallel region
  #pragma omp parallel num_threads(2) 
  {
    ompt_task_id_t implicit_task_id = ompt_get_task_id(0);
    ompt_task_id_t parent_task_id = ompt_get_task_id(1);
    
    #pragma omp critical
    {
     is_task_unique( map_taskID, implicit_task_id, parent_task_id);
     has_the_same_parent(map_siblings, parent_task_id);
    }
     // check the correctness of task id for the nested loop 
    #pragma omp parallel num_threads(2)
    {
	// testing against the serial task id
	ompt_task_id_t inner_task_id = ompt_get_task_id(0);
    	
       #pragma omp critical
       {
	is_task_unique( map_taskID, inner_task_id, implicit_task_id);
    	has_the_same_parent(map_siblings, ompt_get_task_id(1));
       }
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

  omp_set_nested(1);
  test_parallel();

  return 0;
}
