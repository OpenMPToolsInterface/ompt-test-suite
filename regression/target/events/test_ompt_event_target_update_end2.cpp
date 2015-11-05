//****************************************************************************
// macros 
//****************************************************************************

#define DEBUG 0


//****************************************************************************
// include test framework (registering events and general event tests)
//****************************************************************************

#include "target_update_end_framework.cpp"


//*************************************************************************
// test case 2: copy variable from device to host
//*************************************************************************

void update_end_test() {
    int a = 0;
    #pragma omp target data map(alloc: a)
    {
        #pragma omp target
        {
            a = 1;
        }

        // copy value to host 
        #pragma omp target update from(a)


        CHECK(a == 1, IMPLEMENTED_BUT_INCORRECT, "copy variable from device to host not working correctly");
    }

    CHECK(number_begin_events == 1, IMPLEMENTED_BUT_INCORRECT, "copy variable from device to host: number of target_task_begin events with task_type = ompt_target_task_update not as expected (expected %d, observed %d)", 1, number_begin_events);

}
