//****************************************************************************
// macros 
//****************************************************************************

#define DEBUG 0


//****************************************************************************
// include test framework (registering events and general event tests)
//****************************************************************************

#include "target_update_end_framework.cpp"


//*************************************************************************
// test case 6: no update, just simple target data region
// (test if runtime can distinguish target data events and target update
//  events)
//*************************************************************************

void update_end_test() {
    int *x = (int*) malloc(10 * sizeof(int));
    int *y = (int*) malloc(10 * sizeof(int));
    int a = 1;

    #pragma omp target data map(tofrom: x[0:10])
    {
        sleep(1);
    }

    CHECK(number_begin_events == 0, IMPLEMENTED_BUT_INCORRECT, "no update, only target data region: number of update_begin events not as expected (expected %d, observed %d)", 0, number_begin_events);

}
