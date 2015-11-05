//****************************************************************************
// macros 
//****************************************************************************

#define DEBUG 0


//****************************************************************************
// include test framework (registering events and general event tests)
//****************************************************************************

#include "target_update_end_framework.cpp"


//*************************************************************************
// test case 3: copy array from host to device
//*************************************************************************

void update_end_test() {
    int *x = (int*) malloc(10 * sizeof(int));

    #pragma omp target data map(alloc: x[0:10])
    {
        // modify array on host
        for (int i=0; i < 10; i++) {
            x[i] = i;
        }
        
        // copy array to device
        #pragma omp target update to(x[0:10])

        bool device_ok = true;
        #pragma omp target
        { 
            for (int i=0; i < 10; i++) {
                if (x[i] != i)
                    device_ok = false;
            }
        }
        CHECK(device_ok, IMPLEMENTED_BUT_INCORRECT, "copy array from host to device not working correctly");

    }

    CHECK(number_begin_events == 1, IMPLEMENTED_BUT_INCORRECT, "copy array from host to device: number of target_task_begin events with task_type = ompt_target_task_update not as expected (expected %d, observed %d)", 1, number_begin_events);

}
