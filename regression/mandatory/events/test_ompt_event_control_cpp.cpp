#include <omp.h>
#include <common.h>
#include <iostream>
#include <sstream>      

using namespace std;

void on_ompt_event_control(uint64_t command,  uint64_t modifier)
{
    if(command == 101 && modifier == 212) {
        exit(CORRECT);
    }
}

void 
init_test(ompt_function_lookup_t lookup)
{
    if (!register_callback(ompt_event_control, (ompt_callback_t) on_ompt_event_control)) {
        CHECK(false, NOT_IMPLEMENTED, "failed to register ompt_event_control");
    }
}

int
main(int argc, char** argv)
{
    register_segv_handler(argv);
    warmup();
    ompt_control(101, 212);
    usleep(300000);
    return IMPLEMENTED_BUT_INCORRECT;
}
