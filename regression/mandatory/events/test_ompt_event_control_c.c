#include <omp.h>
#include <common.h>


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
        CHECK(FALSE, NOT_IMPLEMENTED, "Failed to register ompt_event_control");
    }
}

int
main(int argc, char** argv)
{
    warmup();
    ompt_control(101, 212);
    usleep(300000);
    return IMPLEMENTED_BUT_INCORRECT;
}
