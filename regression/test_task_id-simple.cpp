#ifndef OMPT_NO_INSTROPECTION

#include <math.h>
#include <iostream>
#include <map>
#include <vector>
#include <assert.h>

#include <omp.h>
#include <ompt.h>
#include <sstream>


/*******************************************************************
 * global variables
 *******************************************************************/

int WAIT_FOR_TASK_ID = 0;
bool error = false;

#define OMPT_FN_TYPE(fn) fn ## _t 
#define OMPT_FN_LOOKUP(lookup,fn) fn = (OMPT_FN_TYPE(fn)) lookup(#fn)
#define OMPT_FN_DECL(fn) OMPT_FN_TYPE(fn) fn

OMPT_FN_DECL(ompt_get_task_id);
OMPT_FN_DECL(ompt_get_thread_id);

typedef std::vector<ompt_task_id_t> TaskVector;

// map of all task id and the parent
typedef std::map<ompt_task_id_t, ompt_task_id_t> MapTaskID;

// list of parent task IDs
static MapTaskID map_taskID;

int ompt_initialize(ompt_function_lookup_t lookup, const char *runtime_version, int ompt_version) {

  /* look up and bind OMPT API functions */
  OMPT_FN_LOOKUP(lookup,ompt_get_task_id);
  OMPT_FN_LOOKUP(lookup,ompt_get_thread_id);
  return 1;
}

#ifdef OMPT_DEBUG
static void 
print_tasks(MapTaskID task_list)
{
  for (MapTaskID::const_iterator it=task_list.begin(); it != task_list.end(); ++it ) 
  {
    std::cout<<"id: " << it->first << " p: "<< it->second << std::endl;
  }
}
#endif

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
      std::cerr << "ERROR: ID already exist: " << id << " vs. " << it->first << std::endl;
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
    std::cerr << "ERROR: task " << ompt_get_task_id(0) << ": " << info << " (IS=" << ID << "; expected=" << expectedID << ")" << std::endl;
    // wait for debugger 
    while (WAIT_FOR_TASK_ID == ompt_get_task_id(0)); 
#else
    assert(ID == expectedID);
#endif
  }
}

void test_region(int level, int maxLevel, int* threadNums, TaskVector parents){
  #pragma omp parallel num_threads(threadNums[level-1]) 
  {
    TaskVector taskIDs;
    #ifdef OMPT_DEBUG
      // Critical section around get_task_id for easier debugging
      #pragma omp critical
    #endif
    for(int i=0; i<=level; i++){
        taskIDs.push_back(ompt_get_task_id(i));
    }
    #pragma omp critical
    {
      #ifdef OMPT_DEBUG
        std::cout <<"(Level " << level << ") implicit_task=" << taskIDs[0] << " enclosing_task=" << taskIDs[1] << std::endl;
      #endif
      is_task_unique( map_taskID, taskIDs[0], taskIDs[1]);
      for(int i=1; i<=level; i++){
        std::stringstream sMsg;
        sMsg << "Parent task ID level " << level << " -> " << level - i << " mismatch";
        assertEqual(taskIDs[i], parents[parents.size()-i], sMsg.str().c_str());
      }
    }
    if(level<maxLevel){
      // Do not modify outer vector as it is used by other threads!
      TaskVector newParents = parents;
      newParents.push_back(taskIDs[0]);
      test_region(level+1, maxLevel, threadNums, newParents);
      // Make sure the task ids did not change
      for(int i=0; i<=level; i++){
        std::stringstream sMsg;
        sMsg << "Task ID level " << level << " -> " << level - i << " changed after nested region";
        assertEqual(ompt_get_task_id(i), taskIDs[i], sMsg.str().c_str());
      }
    }
  }
}

void test_parallel(int nested, int outerThreadNum, int middleThreadNum, int innerThreadNum, int singleThreadNum){

#ifdef OMPT_DEBUG
  std::cout << std::endl << "nested: " << nested 
            << "; outer Threads: " << outerThreadNum 
            << "; middle Threads: " << middleThreadNum 
            << "; inner Threads: " << innerThreadNum
            << "; single (non-nested) Threads: " << singleThreadNum << std::endl; 
#endif
  
  ompt_task_id_t serial_taskID = ompt_get_task_id(0); 
  #ifdef OMPT_DEBUG
    std::cout << "(before first region) serial_thread_task_id=" << serial_taskID << std::endl; 
  #endif
  int threadNums[3];
  threadNums[0]=outerThreadNum; threadNums[1]=middleThreadNum; threadNums[2]=innerThreadNum;
  TaskVector parents;
  parents.push_back(serial_taskID);
  test_region(1, (innerThreadNum)?3:2, threadNums, parents);
  
#ifdef OMPT_DEBUG
  std::cout << std::endl << "(before second region) serial_thread_task_id=" << ompt_get_task_id(0) << std::endl; 
#endif
  assertEqual(ompt_get_task_id(0), serial_taskID, "Serial task id changed after region 1");
  test_region(1, 1, &singleThreadNum, parents);
}


// Tests if nesting of task ids and parents is right for 2 nested sections with different combinations of thread counts
int 
main(int argc, char *argv[])
{
#ifdef OMPT_DEBUG
  if (argc > 2) { std::cerr << "usage: " << argv[0] << "task_id to debug" << std::endl; exit(-1); }
  if (argc == 2) {
    WAIT_FOR_TASK_ID = atoi(argv[1]);
  } 
#endif

  ompt_task_id_t taskid = ompt_get_task_id(0);
  assert(taskid == 0);

  int max_threads = omp_get_max_threads();
  assert(max_threads > 0);

  int n_threads = sqrt(max_threads);
  assert(n_threads > 0);
  
  // Test all possible combinations of nested and inner/outer thread count
  test_parallel(1, 1, 1, 0, 1);
  test_parallel(1, n_threads, 1, 0, 1);
  test_parallel(1, 1, n_threads, 0, max_threads);
  test_parallel(1, n_threads, n_threads, 0, 1);

  test_parallel(0, 1, 1, 0, 1);
  test_parallel(0, n_threads, 1, 0, 1);
  test_parallel(0, 1, n_threads, 0, max_threads);
  test_parallel(0, n_threads, n_threads, 0, 1);
  
  n_threads = 2;
  test_parallel(1, 1, 1, 1, 1);
  test_parallel(1, n_threads, 1, 1, 1);
  test_parallel(1, 1, n_threads, 1, max_threads);
  test_parallel(1, n_threads, n_threads, 1, 1);
  test_parallel(1, 1, 1, n_threads, 1);
  test_parallel(1, n_threads, 1, n_threads, 1);
  test_parallel(1, 1, n_threads, n_threads, max_threads);
  test_parallel(1, n_threads, n_threads, n_threads, 1);
  
  test_parallel(0, 1, 1, 1, 1);
  test_parallel(0, n_threads, 1, 1, 1);
  test_parallel(0, 1, n_threads, 1, max_threads);
  test_parallel(0, n_threads, n_threads, 1, 1);
  test_parallel(0, 1, 1, n_threads, 1);
  test_parallel(0, n_threads, 1, n_threads, 1);
  test_parallel(0, 1, n_threads, n_threads, max_threads);
  test_parallel(0, n_threads, n_threads, n_threads, 1);
  
#ifdef OMPT_DEBUG  
  if(error)
    std::cerr << std::endl << "Test failed!" << std::endl << std::endl;
  else
    std::cerr << std::endl << "Test passed!" << std::endl << std::endl;
#endif

  return 0;
}

#endif
