#include <iostream>
#include <map>
#include <assert.h>

#include <omp.h>
#include <fstream>
#include <sstream>

#include "common.h"

using namespace std;

typedef map<ompt_task_id_t, int> threadIdMap_t;
typedef map<ompt_task_id_t, ompt_parallel_id_t> taskParallelIdMap_t;

threadIdMap_t threadIdMap;
taskParallelIdMap_t taskParallelIdMap;
omp_lock_t mapLock;
//
//Debug output stream
std::ofstream dout("/dev/null");


static void 
on_thread_begin(ompt_thread_type_t thread_type, ompt_thread_id_t thread_id){

  dout << "on_thread_begin " << thread_type << "  id: " << thread_id << endl;

  if (omp_get_thread_num() == 0) {
    // master thread must be initial thread
    assert(thread_type == ompt_thread_initial);
  } else {
    // if it is not master, it must be worker thread
    assert(thread_type == ompt_thread_worker);

    // the thread id must be unique
    assert(threadIdMap.count(thread_id)==0);
    threadIdMap[thread_id] = thread_type;
  }
}

static void 
on_thread_end(ompt_thread_type_t thread_type, ompt_thread_id_t thread_id){

  dout << "on_thread_end " << thread_type << "  id: " << thread_id << endl;

  // for the end of the thread, only worker threads is invoked
  assert(thread_type == ompt_thread_worker);

  // the thread id should be the same as the thread id in the thread_begin
  assert(threadIdMap.count(thread_id)>0);
}


static void
on_parallel_begin(  ompt_task_id_t parent_task_id,    /* id of parent task            */
  ompt_frame_t *parent_task_frame,  /* frame data of parent task    */
  ompt_parallel_id_t parallel_id,   /* id of parallel region        */
  uint32_t requested_team_size,     /* number of threads in team    */
  void *parallel_function           /* pointer to outlined function */)
{
  // check the size of the team
  dout<<"\ton_parallel_begin : " << parent_task_id<< ", frame: " << parent_task_frame
	<<", par id: " << parallel_id<< ", size : " << requested_team_size
	<<", fct: " << parallel_function<< endl; 
  
  assert(requested_team_size == 2);
  taskParallelIdMap[parallel_id] = parent_task_id;
}

static void
on_parallel_end(ompt_parallel_id_t parallel_id,    /* id of parallel region       */
  		  ompt_task_id_t task_id             /* id of task                  */)
{
  dout << "\ton_parallel_end: " << parallel_id << ", task: " << task_id<< endl;

  // check if the parallel id has been defined in the parallel begin event
  assert(taskParallelIdMap.count(parallel_id)==1);

  omp_set_lock(&mapLock);
  taskParallelIdMap.erase(parallel_id);
  omp_unset_lock(&mapLock);
}

void init_test(){
  REG_CB(parallel_begin)
  REG_CB(parallel_end)
  REG_CB(thread_begin)
  REG_CB(thread_end)
}


void test_loops(){
  int ct = 100000000;
  float* list = (float*) malloc(sizeof(float)*ct);

  #pragma omp parallel num_threads(2) 
  {
    #pragma omp parallel num_threads(2)
    {
      #pragma omp for 
      for(int i=0; i<ct; i++)
      	// initialize the list. race condition is okay
      	list[i] *= 1.f/i; 
      }
  }
  free(list);
}

int main(int argc, char** argv) {
  
#ifdef OMPT_DEBUG
  ostream& dout_ref = dout;
  dout_ref.rdbuf(cout.rdbuf());
#endif
  omp_init_lock(&mapLock);
  // enable nested parallelism
  omp_set_nested(1);

  test_loops();

  // make sure that all parallel region ID has been checked
  assert(taskParallelIdMap.size()==0);
  omp_destroy_lock(&mapLock);

  return 0;
}

