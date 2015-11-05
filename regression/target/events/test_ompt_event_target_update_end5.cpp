//****************************************************************************
// macros 
//****************************************************************************

#define DEBUG 0


//****************************************************************************
// include test framework (registering events and general event tests)
//****************************************************************************

#include "target_update_end_framework.cpp"


//*************************************************************************
// test case 5: copy array and variable in both directions
//*************************************************************************

void update_end_test() {
    int a = 1;
    int *x = (int*) malloc(10*sizeof(int));

    for (int i=0; i < 10; i++) {
        x[i] = i;
    }

    #pragma omp target data map (alloc: a, x[0:10])
    {
        // copy both variable and array to device
        #pragma omp target update to(a, x[0:10])

        bool device_ok = true;
        #pragma omp target
        {
            if (a != 1)
                device_ok = false;

            for (int i=0; i < 10; i++) {
                if (x[i] != i)
                    device_ok = false;
            }
        }
        CHECK(device_ok, IMPLEMENTED_BUT_INCORRECT, "copy from array and variable from host to device not working");

        #pragma omp target
        {
            // modify variable and array on device
            a = 2;

            for (int i=0; i < 10; i++) {
                x[i] = i+1;
            }
        }

        // copy both variable and array to host
        #pragma omp target update from(a, x[0:10])

        bool host_ok = true;

        if (a != 2)
            host_ok = false;

        for (int i=0; i < 10; i++) {
            if (x[i] != i+1)
                host_ok = false;
        }

        CHECK(host_ok, IMPLEMENTED_BUT_INCORRECT, "copy from array and variable from device to host not working");
        
    }
  
    CHECK(number_begin_events == 2, IMPLEMENTED_BUT_INCORRECT, "copy array and variable in both directions: number of target_task_begin events with task_type = ompt_target_task_update not as expected (expected %d, observed %d)", 2, number_begin_events);

}
