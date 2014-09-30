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
bool initial = true;
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
    threadIdMap[thread_id] = 0;
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
  
}

static void
on_parallel_end(ompt_parallel_id_t parallel_id,    /* id of parallel region       */
  		  ompt_task_id_t task_id             /* id of task                  */)
{
  dout << "\ton_parallel_end: " << parallel_id << ", task: " << task_id<< endl;
}

void init_test(){
  REG_CB(parallel_begin)
  REG_CB(parallel_end)
  REG_CB(thread_begin)
  REG_CB(thread_end)
}

void run_loop(int numThreads){
  int ct = 100000000;
  float* list = (float*) malloc(sizeof(float)*ct);
  
  dout << "\t\trun_loop " << numThreads << endl;
  #pragma omp parallel for num_threads(numThreads)
  for(int i=0; i<ct; i++)
    list[i] *= 1.f/i; 
  free(list);
}

void test_loops(){
  run_loop(omp_get_max_threads());
  initial = false;
  
  int threads = omp_get_max_threads()/2;
  threads  = (threads>0 ? threads : 1);
  #pragma omp parallel num_threads(2)
  {
    run_loop(threads);  
  }
}

int main(int argc, char** argv) {
  
#ifdef OMPT_DEBUG
  ostream& dout_ref = dout;
  dout_ref.rdbuf(cout.rdbuf());
#endif
  test_loops();
  
  return 0;
}

