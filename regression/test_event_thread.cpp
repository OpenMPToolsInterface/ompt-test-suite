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

typedef map<ompt_task_id_t, int> threadIdMap_t;

static threadIdMap_t threadIdMap;


void on_thread_begin(ompt_thread_type_t thread_type, ompt_thread_id_t thread_id){
  if (omp_get_thread_num() == 0) {
    assert(thread_type == ompt_thread_initial);
  } else {
    assert(thread_type == ompt_thread_worker);
    assert(threadIdMap.count(thread_id)==0);
    threadIdMap[thread_id] = 0;
  }
}

void on_thread_end(ompt_thread_type_t thread_type, ompt_thread_id_t thread_id){
  assert(thread_type == ompt_thread_worker);
  assert(threadIdMap.count(thread_id)>0);
}


void init_test(){
  REG_CB(thread_begin)
  REG_CB(thread_end)
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
  run_loop(omp_get_max_threads());
  
  int threads = omp_get_max_threads()/2;
  #pragma omp parallel num_threads(2)
  {
    run_loop(threads);  
  }
}

int main(int argc, char** argv) {
  
  test_loops();
  
  return 0;
}

