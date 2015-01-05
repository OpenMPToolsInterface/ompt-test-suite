#include <omp.h>
#include <common.h>
#include <vector>

#define DEBUG 0

#define NUM_THREADS 4

using namespace std;

// target_begin -> increased, target_end -> decreased
int count = 0; 

// save target_id and corresponding task_id for a target_begin (for each thread)
vector<ompt_target_id_t> target_ids(NUM_THREADS, 0);
vector<ompt_task_id_t> task_ids(NUM_THREADS, 0);

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    
void on_ompt_event_target_begin(ompt_task_id_t task_id,
                ompt_target_id_t target_id,
                ompt_target_device_id_t device_id,
                void* target_function) {
    pthread_mutex_lock(&mutex);

#if DEBUG
    printf("begin: task_id = %lld, target_id = %lld, device_id = %lld, target_function = %p\n",
        task_id, target_id, device_id, target_function);
#endif

    CHECK(task_id > 0, IMPLEMENTED_BUT_INCORRECT, "invalid task_id");
    CHECK(task_id == ompt_get_task_id(0), IMPLEMENTED_BUT_INCORRECT, "task_id not equal to ompt_get_task_id()");

    // save task_id and target_id for current thread
    target_ids[ompt_get_thread_id()] = target_id;
    task_ids[ompt_get_thread_id()] = task_id;

    count += 1;

    pthread_mutex_unlock(&mutex);
}

void on_ompt_event_target_end(ompt_task_id_t task_id,
                  ompt_target_id_t target_id) {
    pthread_mutex_lock(&mutex);

#if DEBUG
    printf("end: task_id = %lld, target_id = %lld\n", task_id, target_id);
#endif

    CHECK(task_id > 0, IMPLEMENTED_BUT_INCORRECT, "invalid task_id");
    CHECK(task_id == ompt_get_task_id(0), IMPLEMENTED_BUT_INCORRECT, "task_id not equal to ompt_get_task_id()");

    // check for correct target_id and task_id in target_end
    // (should be the same as in target_begin for current thread)
    CHECK(target_ids[ompt_get_thread_id()] == target_id, IMPLEMENTED_BUT_INCORRECT, "target_ids not equal");
    CHECK(task_ids[ompt_get_thread_id()] == task_id, IMPLEMENTED_BUT_INCORRECT, "task_ids not equal"); 

    count -= 1;

    pthread_mutex_unlock(&mutex);
}

void init_test(ompt_function_lookup_t lookup) {
    if (!register_callback(ompt_event_target_begin, (ompt_callback_t) on_ompt_event_target_begin)) {
        CHECK(false, FATAL, "failed to register ompt_event_target_begin");
    }

    if (!register_callback(ompt_event_target_end, (ompt_callback_t) on_ompt_event_target_end)) {
        CHECK(false, FATAL, "failed to register ompt_event_target_end");
    }
}


int main(int argc, char** argv) {
    register_segv_handler(argv);

    #pragma omp parallel num_threads(NUM_THREADS)
    {
        #pragma omp target  
        {
            sleep(1);
        }

        #pragma omp target    
        {
            sleep(1);
        }
    }

    CHECK(count == 0, IMPLEMENTED_BUT_INCORRECT,  "not the same number of target_begin and target_end calls");

    return global_error_code;
}

