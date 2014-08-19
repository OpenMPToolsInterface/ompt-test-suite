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

#define OMPT_FN_TYPE(fn) fn ## _t 
#define OMPT_FN_LOOKUP(lookup,fn) fn = (OMPT_FN_TYPE(fn)) lookup(#fn)
#define OMPT_FN_DECL(fn) OMPT_FN_TYPE(fn) fn

OMPT_FN_DECL(ompt_get_task_id);

typedef std::vector<ompt_task_id_t> TaskVector;

// map of all task id and the parent
typedef std::map<ompt_task_id_t, ompt_task_id_t> MapTaskID;

// list of parent task IDs
static MapTaskID map_taskID, map_siblings;

int ompt_initialize(ompt_function_lookup_t lookup, const char *runtime_version, int ompt_version) {

  /* look up and bind OMPT API functions */
  OMPT_FN_LOOKUP(lookup,ompt_get_task_id);
}


static void 
print_tasks(MapTaskID task_list)
{
  for (MapTaskID::const_iterator it=task_list.begin(); it != task_list.end(); ++it ) 
  {
    std::cout<<"id: " << it->first << " p: "<< it->second << std::endl;
  }
}


static bool
task_push(MapTaskID &task_list, ompt_task_id_t id, ompt_task_id_t parent)
{
  if (task_list.size()>0)
  {
    MapTaskID::const_iterator it = task_list.find(id);
    // id must be unique
    if ( it != task_list.end() ) {
	std::cout<<" ID already exist: " << id << " vs. " << it->first << std::endl;
	assert(false);
    }
  }
  #pragma omp critical
  {
    task_list[id] = parent;
  }
}


static bool
task_match(MapTaskID &task_list, ompt_task_id_t id, ompt_task_id_t parent)
{
  if (task_list.size()>0) {
    MapTaskID::const_iterator it = task_list.find(id);
    // id must be unique
    if ( it != task_list.end()) {
        if  (it->second != parent) {
	  std::cout <<" siblings don't have the same parent: " << it->second << " vs. " << parent  << std::endl;
	  assert(false);
        } else {
          return true;
       }
    }
  }
  #pragma omp critical
  {
    task_list[id] = parent;
  }
}

void test_parallel(int nested, int max_threads, int n_threads)
{
  std::cout << "(before first region) serial_thread_task_id=" << ompt_get_task_id(0) << std::endl; 

  omp_set_nested(nested);
  #pragma omp parallel num_threads(n_threads) 
  {
   	  #pragma omp critical
          {
	  std::cout <<"(outer region) implicit_task=" << ompt_get_task_id(0) << 
	  	" enclosing_task=" << ompt_get_task_id(1) << std::endl;
          }
    task_push( map_taskID, ompt_get_task_id(0), ompt_get_task_id(1));
    task_match(map_siblings, 1, ompt_get_task_id(1));

    ompt_task_id_t my_outer_task = ompt_get_task_id(0); 

    #pragma omp parallel num_threads(n_threads)
    {
#ifdef OMPT_DEBUG
	if (my_outer_task != ompt_get_task_id(1)) {
   	  #pragma omp critical
          {
	  std::cout <<"task " << ompt_get_task_id(0) <<": my enclosing_task_id=" << my_outer_task << " != ompt_get_task_id(1)=" 
		<< ompt_get_task_id(1)  << std::endl;
          }
	  // wait for debugger 
	  while (WAIT_FOR_TASK_ID == ompt_get_task_id(0)); 
        }
#else
        assert( my_outer_task == ompt_get_task_id(1));
#endif
	task_push( map_taskID, ompt_get_task_id(0), ompt_get_task_id(1));
        task_match(map_siblings, my_outer_task, ompt_get_task_id(1));
    }
  }

  std::cout << "(before second region) serial_thread_task_id=" << ompt_get_task_id(0) << std::endl; 

  omp_set_num_threads(max_threads);
  #pragma omp parallel for
  for(int i=0; i<4; i++)
  {
    task_push( map_taskID, ompt_get_task_id(0), ompt_get_task_id(1));
    task_match(map_siblings, 2, ompt_get_task_id(1));
  }

  std::cout << "(after second region) serial_thread_task_id=" << ompt_get_task_id(0) << std::endl; 

#ifdef OMPT_DEBUG
  print_tasks(map_taskID);
#endif
}


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
  std::cout << "max threads: " << max_threads << " nth: " << n_threads << std::endl; 
  
  test_parallel(1, max_threads, n_threads);
  test_parallel(0, max_threads, n_threads);

  return 0;
}

#endif
