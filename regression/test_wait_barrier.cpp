#include <math.h>
#include <iostream>
#include <map>
#include <vector>
#include <assert.h>

#include <omp.h>
#include <fstream>
#include <sstream>

#include "common.h"

using namespace std;

typedef map<ompt_task_id_t, ompt_parallel_id_t> taskPidMap_t;

taskPidMap_t taskPidMap;
omp_lock_t mapLock;

//Debug output stream
std::ofstream dout("/dev/null");

void on_wait_barrier_begin(ompt_parallel_id_t parallel_id, ompt_task_id_t task_id){
  omp_set_lock(&mapLock);
  if(taskPidMap.count(task_id)){
    cerr << "Got a wait_barrier_begin for an already waiting task" << endl;
    assert(false);
  }
  taskPidMap[task_id] = parallel_id;
  omp_unset_lock(&mapLock);
}

void on_wait_barrier_end(ompt_parallel_id_t parallel_id, ompt_task_id_t task_id){
  omp_set_lock(&mapLock);
  if(!taskPidMap.count(task_id)){
    cerr << "Got a wait_barrier_end for a task that is not waiting" << endl;
    assert(false);
  }
  if(taskPidMap[task_id]!=parallel_id){
    cerr << "Got a wait_barrier_end for a task that waiting in another parallel region:" << endl
         << "Expected: " << taskPidMap[task_id] << " Is:" << parallel_id << endl;
    assert(false);
  }
  taskPidMap.erase(task_id);
  omp_unset_lock(&mapLock);
}

void assert_empty_map(){
  if(taskPidMap.size()){
      cerr << "Have " << taskPidMap.size() << " waiting tasks after parallel region(s)" <<endl;
      assert(false);
  }
}

int init_test(ompt_function_lookup_t lookup,
	     const char*            runtime_version,
	     unsigned int           ompt_version ) 
{
  dout << "Init test" << endl;
  REG_CB(wait_barrier_begin)
  REG_CB(wait_barrier_end)

  return 1; // tool present
}

void run_loop(int numThreads){
  int ct = 100000000;
  float* list = (float*) malloc(sizeof(float)*ct);
  #pragma omp parallel for num_threads(numThreads)
  for(int i=0; i<ct; i++)
    list[i] *= 1.f/i; 
  free(list);
}

void test_loops(){
  dout << "Before 1st loop" << endl;
  run_loop(omp_get_max_threads());
  assert_empty_map();
  dout << "After 1st loop" << endl;
  
  
  int threads = omp_get_max_threads()/2;
  #pragma omp parallel num_threads(2)
  {
    run_loop(threads);  
  }
  assert_empty_map();
  dout << "After 2nd loop" << endl;
}

void run_test(int nested){
  dout << "Run test with nested=" << nested << endl;
  omp_set_nested(nested);
  
  test_loops();
  
  int threads = omp_get_max_threads()/2;
  #pragma omp parallel num_threads(2)
  {
    #pragma omp critical
    dout << "Start in region tid=" << ompt_get_thread_id() << endl;
    #pragma omp master
    {
      #pragma omp task
      {
        run_loop(threads);
        dout << "End task1 TID=" << ompt_get_thread_id() << endl;
      }
      #pragma omp task
      {
        run_loop(threads);
        dout << "End task2 TID=" << ompt_get_thread_id() << endl;
      }
    }
  }
  assert_empty_map();
}

int main(int argc, char** argv) {
  omp_init_lock(&mapLock);
#ifdef OMPT_DEBUG
  ostream& dout_ref = dout;
  dout_ref.rdbuf(cout.rdbuf());
#endif
  
  run_test(0);
  run_test(1);
  
  omp_destroy_lock(&mapLock);
  return 0;
}

