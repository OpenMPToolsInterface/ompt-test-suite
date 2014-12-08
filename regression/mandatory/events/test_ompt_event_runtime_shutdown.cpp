#include <omp.h>
#include <common.h>
#include <iostream>
#include <sstream>      

using namespace std;

void on_ompt_event_runtime_shutdown(uint64_t command,  uint64_t modifier)
{
    printf("shutdown\n");
    exit(CORRECT);
}

void 
init_test(ompt_function_lookup_t lookup)
{
    if (!register_callback(ompt_event_runtime_shutdown, (ompt_callback_t) on_ompt_event_runtime_shutdown)) {
        cout << "Failed to register ompt_event_runtime_shutdown" << endl;
        exit(NOT_IMPLEMENTED);
    }
}

void work()
{
    #pragma parallel num_threads(NUM_THREADS)
    {
        serialwork(1);
    }
}

int
main(int argc, char** argv)
{
    work();
    exit(IMPLEMENTED_BUT_INCORRECT);
}
