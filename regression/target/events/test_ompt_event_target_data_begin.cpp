#include <omp.h>
#include <common.h>
#include <stdlib.h>

#define DEBUG 1

int number_begin_events = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void on_ompt_event_target_data_begin(ompt_task_id_t task_id,
                ompt_target_id_t target_id,
                ompt_target_device_id_t device_id,
                void* target_function) {
    pthread_mutex_lock(&mutex);

#if DEBUG
    printf("task_id = %lld, target_id = %lld, device_id = %lld, target_function = %p\n",
        task_id, target_id, device_id, target_function);
#endif

    CHECK(task_id > 0, IMPLEMENTED_BUT_INCORRECT, "invalid task_id");
    CHECK(task_id == ompt_get_task_id(0), IMPLEMENTED_BUT_INCORRECT, "task_id not equal to ompt_get_task_id()");    

    number_begin_events += 1;

    pthread_mutex_unlock(&mutex);
} 

void init_test(ompt_function_lookup_t lookup)
{
    if (!register_callback(ompt_event_target_data_begin, (ompt_callback_t) on_ompt_event_target_data_begin)) {
        CHECK(false, FATAL, "failed to register ompt_event_target_update_begin");
    }
}

int main(int argc, char** argv) {
    register_segv_handler(argv);

    // task_id=0 workaround
    // TODO: fix in OMPT implementation
    #pragma omp parallel    
    {
    }


    int a = 1;
    int *x = (int*) malloc(10 * sizeof(int));
    #pragma omp target data map(tofrom: x[0:10], a)
    {
       sleep(1);
    }

    CHECK(number_begin_events == 1, IMPLEMENTED_BUT_INCORRECT, "number of data_begins does not match with number of data regions (expected %d, observed %d)", 1, number_begin_events);

    return global_error_code;
}
