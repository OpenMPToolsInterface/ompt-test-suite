//****************************************************************************
// macros 
//****************************************************************************

#define DEBUG 0


//****************************************************************************
// include test framework (registering events and general event tests)
//****************************************************************************

#include "target_data_end_framework.cpp"


// ***********************************************************************
// test case 4: copy (tofrom) array and variable together
// ***********************************************************************

void data_end_test() {
    int *z = (int*) malloc(10 * sizeof(int));
    int a = 1;

    #pragma omp target data map(tofrom: z[0:10], a)
    {
        sleep(1);
    }

    CHECK(number_begin_events == 1, IMPLEMENTED_BUT_INCORRECT, "number of data_begin events does not match with number of data regions (expected %d, observed %d)", 1, number_begin_events);
}
