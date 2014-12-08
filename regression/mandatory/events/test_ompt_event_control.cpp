#include <omp.h>
#include <common.h>
#include <iostream>
#include <sstream>      

using namespace std;

void on_ompt_event_control(uint64_t command,  uint64_t modifier)
{
    ASSERT(command == 101 && modifier == 212, IMPLEMENTED_BUT_INCORRECT, "Wrong arguments passed to control callback");
    exit(CORRECT);
}

void 
init_test(ompt_function_lookup_t lookup)
{
    if (!register_callback(ompt_event_control, (ompt_callback_t) on_ompt_event_control)) {
        cout << "Failed to register ompt_event_control" << endl;
        exit(NOT_IMPLEMENTED);
    }
}

int
main(int argc, char** argv)
{
    ompt_control(101, 212);
    for (;;) pause();
    return CORRECT;
}
