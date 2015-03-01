//****************************************************************************
// macros 
//****************************************************************************

#define DEBUG 0


//****************************************************************************
// include test framework (registering events and general event tests)
//****************************************************************************

#include "target_update_end_framework.cpp"


//*************************************************************************
// test case 1: copy variable from host to device
//*************************************************************************

void update_end_test() {
    int a = 0;
    #pragma omp target data map(alloc: a)
    {
        // modify value on host
        a = 1;

        // copy value to device
        #pragma omp target update to(a)

        bool device_ok = true;
        #pragma omp target
        {
            if (a != 1)
                device_ok = false;     
        }
        CHECK(device_ok, IMPLEMENTED_BUT_INCORRECT, "copy variable from host to device not working correctly");
    }

    CHECK(number_begin_events == 1, IMPLEMENTED_BUT_INCORRECT, "copy variable from host to device: number of update_begin events not as expected (expected %d, observed %d)", 1, number_begin_events);

}
