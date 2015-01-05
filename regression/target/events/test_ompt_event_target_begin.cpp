#include <omp.h>
#include <common.h>

#define DEBUG 0

void on_ompt_event_target_begin(ompt_task_id_t task_id,
                ompt_target_id_t target_id,
                ompt_target_device_id_t device_id,
                void* target_function) {
#if DEBUG
    printf("task_id = %lld, target_id = %lld, device_id = %lld, target_function = %p\n",
        task_id, target_id, device_id, target_function);
#endif

    CHECK(task_id > 0, IMPLEMENTED_BUT_INCORRECT, "invalid task_id");
    CHECK(task_id == ompt_get_task_id(0), IMPLEMENTED_BUT_INCORRECT, "task_id not equal to ompt_get_task_id()");
}

void init_test(ompt_function_lookup_t lookup)
{
    if (!register_callback(ompt_event_target_begin, (ompt_callback_t) on_ompt_event_target_begin)) {
        CHECK(false, FATAL, "failed to register ompt_event_target_begin");
    }
}

int main(int argc, char** argv) {
    register_segv_handler(argv);

    // task_id=0 workaround
    // TODO: fix in OMPT implementation
    #pragma omp parallel    
    {
    }

    #pragma omp target device(0) 
    {
        sleep(1);
    }

    return global_error_code;
}
