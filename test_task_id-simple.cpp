#ifndef OMPT_NO_INSTROPECTION

#include <math.h>
#include <iostream>
#include <map>
#include <vector>
#include <assert.h>

#include <omp.h>
#include <ompt.h>


/*******************************************************************
 * global variables
 *******************************************************************/

int WAIT_FOR_TASK_ID = 0;
bool error = false;

#define OMPT_FN_TYPE(fn) fn ## _t 
#define OMPT_FN_LOOKUP(lookup,fn) fn = (OMPT_FN_TYPE(fn)) lookup(#fn)
#define OMPT_FN_DECL(fn) OMPT_FN_TYPE(fn) fn

OMPT_FN_DECL(ompt_get_task_id);

typedef std::vector<ompt_task_id_t> TaskVector;

// map of all task id and the parent
typedef std::map<ompt_task_id_t, ompt_task_id_t> MapTaskID;

// list of parent task IDs
static MapTaskID map_taskID;

int ompt_initialize(ompt_function_lookup_t lookup, const char *runtime_version, int ompt_version) {

  /* look up and bind OMPT API functions */
  OMPT_FN_LOOKUP(lookup,ompt_get_task_id);
  return 1;
}


static void 
print_tasks(MapTaskID task_list)
{
  for (MapTaskID::const_iterator it=task_list.begin(); it != task_list.end(); ++it ) 
  {
    std::cout<<"id: " << it->first << " p: "<< it->second << std::endl;
  }
}


// Test if the task id was already used
// Has to be called from inside critical section
static bool
is_task_unique(MapTaskID &task_list, ompt_task_id_t id, ompt_task_id_t parent)
{
  if (task_list.size()>0)
  {
    MapTaskID::const_iterator it = task_list.find(id);
    // id must be unique
    if ( it != task_list.end() ) {
      std::cout<<" ID already exist: " << id << " vs. " << it->first << std::endl;
      error = true;
      assert(false);
    }
  }
  task_list[id] = parent;
  return true;
}

// Checks if the ID matches the expected ID
// Better to call this from inside critical section
static void assertEqual(ompt_task_id_t ID, ompt_task_id_t expectedID, const char* info){
  if (ID != expectedID) {
    error = true;
#ifdef OMPT_DEBUG
    std::cout << "task " << ompt_get_task_id(0) << ": " << info << " (IS=" << ID << "; expected=" << expectedID << ")" << std::endl;
    // wait for debugger 
    while (WAIT_FOR_TASK_ID == ompt_get_task_id(0)); 
#else
    assert(ID == expectedID);
#endif
  }
}

void test_parallel(int nested, int outerThreadNum, int innerThreadNum, int singleThreadNum)
{
  std::cout << "nested: " << nested << "; outer Threads: " << outerThreadNum << "; inner Threads: " << innerThreadNum
            << "; single (non-nested) Threads: " << singleThreadNum << std::endl; 

  ompt_task_id_t serial_taskID = ompt_get_task_id(0); 
#ifdef OMPT_DEBUG
  std::cout << "(before first region) serial_thread_task_id=" << serial_taskID << std::endl; 
#endif

  omp_set_nested(nested);
  #pragma omp parallel num_threads(outerThreadNum) 
  {
    ompt_task_id_t outer_taskID = ompt_get_task_id(0);
    ompt_task_id_t outer_taskID_parent = ompt_get_task_id(1);
        
#ifdef OMPT_DEBUG
    #pragma omp critical
    {
      std::cout <<"(outer region) implicit_task=" << outer_taskID << " enclosing_task=" << outer_taskID_parent << std::endl;
    }
#endif
    #pragma omp critical
    {
      is_task_unique( map_taskID, outer_taskID, outer_taskID_parent);
      assertEqual(outer_taskID_parent, serial_taskID, "Parent task ID level 1 mismatch");
    }

    #pragma omp parallel num_threads(innerThreadNum)
    {
#ifdef OMPT_DEBUG
      // Critical section also around the get task id for easier debugging
      #pragma omp critical
      {
#endif
        ompt_task_id_t inner_taskID = ompt_get_task_id(0); 
        ompt_task_id_t inner_taskID_parent = ompt_get_task_id(1); 
        ompt_task_id_t inner_taskID_grandParent = ompt_get_task_id(2); 

#ifdef OMPT_DEBUG
        std::cout <<"(inner region) implicit_task=" << inner_taskID << " enclosing_task=" << inner_taskID_parent << std::endl;
#else
      #pragma omp critical
      {
#endif
        is_task_unique( map_taskID, inner_taskID, inner_taskID_parent);
        
        assertEqual(inner_taskID_parent, outer_taskID, "Parent task ID level 2 mismatch");
        assertEqual(inner_taskID_grandParent, serial_taskID, "Grandparent task ID level 2 mismatch");
      }
    }
  }

#ifdef OMPT_DEBUG
  std::cout << std::endl << "(before second region) serial_thread_task_id=" << ompt_get_task_id(0) << std::endl; 
#endif
  assertEqual(ompt_get_task_id(0), serial_taskID, "Serial task id changed after region 1");

  #pragma omp parallel num_threads(singleThreadNum)
  {
    ompt_task_id_t outer_taskID = ompt_get_task_id(0);
    ompt_task_id_t outer_taskID_parent = ompt_get_task_id(1);
    
    #pragma omp critical
    {
      is_task_unique( map_taskID, outer_taskID, outer_taskID_parent);
      assertEqual(outer_taskID_parent, serial_taskID, "Parent task ID level 1 mismatch");
    }
  }

  assertEqual(ompt_get_task_id(0), serial_taskID, "Serial task id changed after region 2");
#ifdef OMPT_DEBUG
  std::cout << "(after second region) serial_thread_task_id=" << serial_taskID << std::endl; 
  print_tasks(map_taskID);
#endif
  std::cout << std::endl;
}


// Tests if nesting of task ids and parents is right for 2 nested sections with different combinations of thread counts
int 
main(int argc, char *argv[])
{
  if (argc > 2) { std::cerr << "usage: " << argv[0] << "task_id to debug" << std::endl; exit(-1); }
  if (argc == 2) {
    WAIT_FOR_TASK_ID = atoi(argv[1]);
  } 
  ompt_task_id_t taskid = ompt_get_task_id(0);
  assert(taskid == 0);

  int max_threads = omp_get_max_threads();
  assert(max_threads > 0);

  int n_threads = sqrt(max_threads);
  assert(n_threads > 0);
  
  // Test all possible combinations of nested and inner/outer thread count
  test_parallel(1, 1, 1, 1);
  test_parallel(1, n_threads, 1, 1);
  test_parallel(1, 1, n_threads, max_threads);
  test_parallel(1, n_threads, n_threads, 1);

  test_parallel(0, 1, 1, 1);
  test_parallel(0, n_threads, 1, 1);
  test_parallel(0, 1, n_threads, max_threads);
  test_parallel(0, n_threads, n_threads, 1);
  
  if(error)
    std::cerr << "Test failed!";
  else
    std::cout << "Test passed!";

  return 0;
}

#endif
