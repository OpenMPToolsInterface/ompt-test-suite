//****************************************************************************
// macros 
//****************************************************************************

#define DEBUG 0


//****************************************************************************
// include test framework (registering events and general event tests)
//****************************************************************************

#include "target_update_end_framework.cpp"


//*************************************************************************
// test case 7: no update, nested target data regions
// (test if runtime can distinguish target data events and target update
//  events)
//*************************************************************************

void update_end_test() {
    int *x = (int*) malloc(10 * sizeof(int));
    int a = 1;
  
    #pragma omp target data map(tofrom: x[0:10]) map(tofrom: a)
    {
        sleep(2);
    }

    CHECK(number_begin_events == 0, IMPLEMENTED_BUT_INCORRECT, "no update, nested target data region: number of target_task_begin events with task_type = ompt_target_task_update not as expected (expected %d, observed %d)", 0, number_begin_events);

}
