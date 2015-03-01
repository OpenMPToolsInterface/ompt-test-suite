//****************************************************************************
// macros 
//****************************************************************************

#define DEBUG 0


//****************************************************************************
// include test framework (registering events and general event tests)
//****************************************************************************

#include "target_data_end_framework.cpp"


// ************************************************************************
// test case 1: empty data region
// ************************************************************************

void data_end_test() {
    #pragma omp target data
    {
        sleep(1);
    }

    CHECK(number_begin_events == 1, IMPLEMENTED_BUT_INCORRECT, "number of data_begin events does not match with number of data regions (expected %d, observed %d)", 1, number_begin_events);
}
