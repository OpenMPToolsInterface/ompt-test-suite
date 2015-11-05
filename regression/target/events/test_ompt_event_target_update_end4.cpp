//****************************************************************************
// macros 
//****************************************************************************

#define DEBUG 0


//****************************************************************************
// include test framework (registering events and general event tests)
//****************************************************************************

#include "target_update_end_framework.cpp"


//*************************************************************************
// test case 4: copy array from device to host
//*************************************************************************

void update_end_test() {
    int *x = (int*) malloc(10 * sizeof(int));

    #pragma omp target data map(alloc: x[0:10])
    {
        #pragma omp target
        { 
            // modify array on device 
            for (int i=0; i < 10; i++) {
                x[i] = i;
            }
           
        }
 
        // copy array to host 
        #pragma omp target update from(x[0:10])


        bool host_ok = true;
        for (int i=0; i < 10; i++) {
            if (x[i] != i)
                host_ok = false;
        }
        CHECK(host_ok, IMPLEMENTED_BUT_INCORRECT, "copy array from device to host not working correctly");
    }

    CHECK(number_begin_events == 1, IMPLEMENTED_BUT_INCORRECT, "copy array from device to host: number of target_task_begin events with task_type = ompt_target_task_update not as expected (expected %d, observed %d)", 1, number_begin_events);

}
