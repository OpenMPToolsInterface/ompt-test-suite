//*****************************************************************************
// system includes
//*****************************************************************************

#include <map>



//*****************************************************************************
// OpenMP runtime includes
//*****************************************************************************

#include <omp.h>



//*****************************************************************************
// regression harness includes
//*****************************************************************************

#include <ompt-regression.h>
#include <ompt-initialize.h>



//*****************************************************************************
// macros
//*****************************************************************************

#define DEBUG 0
#define NUM_THREADS 2



//*****************************************************************************
// global variables
//*****************************************************************************
std::map<ompt_task_id_t, ompt_parallel_id_t> task_id_to_parallel_id_map;

int loop_begin = 0;
int loop_end = 0;


//*****************************************************************************
// private operations 
//*****************************************************************************

static void
on_ompt_event_loop_begin(ompt_parallel_id_t parallel_id,
                         ompt_task_id_t task_id,
                         void *loop_function)
{
    pthread_mutex_lock(&thread_mutex);
#if DEBUG
    printf("loop_begin %lu (parallel %lu)\n", task_id, parallel_id);
#endif

    ompt_parallel_id_t inquiry_parallel_id = ompt_get_parallel_id(0);
    CHECK(parallel_id == inquiry_parallel_id, IMPLEMENTED_BUT_INCORRECT, \
        "parallel id from parameter does not match inquiry function (%lu vs %lu)", \
        parallel_id, inquiry_parallel_id);

    ompt_task_id_t inquiry_task_id = ompt_get_task_id(0);
    CHECK(task_id == inquiry_task_id, IMPLEMENTED_BUT_INCORRECT, \
        "task id from parameter does not match inquiry function (%lu vs %lu)", \
        task_id, inquiry_task_id);

    if (task_id_to_parallel_id_map.count(task_id) == 0) {
        task_id_to_parallel_id_map[task_id] = parallel_id;

        #pragma omp atomic update
        loop_begin += 1;
    } else {
        CHECK(FALSE, IMPLEMENTED_BUT_INCORRECT, \
            "duplicate task id %lu", task_id);
    }

    pthread_mutex_unlock(&thread_mutex);
}

static void
on_ompt_event_loop_end(ompt_parallel_id_t parallel_id,
                       ompt_task_id_t task_id)
{
    pthread_mutex_lock(&thread_mutex);
#if DEBUG
    printf("loop_end %lu (parallel %lu)\n", task_id, parallel_id);
#endif

    if (task_id_to_parallel_id_map.count(task_id) == 1) {
        CHECK(task_id_to_parallel_id_map[task_id] == parallel_id,
            IMPLEMENTED_BUT_INCORRECT, \
            "unmatching task and parallel id (%lu in %lu)", task_id, parallel_id);

        task_id_to_parallel_id_map.erase(task_id);

        #pragma omp atomic update
        loop_end += 1;
    } else {
        CHECK(FALSE, IMPLEMENTED_BUT_INCORRECT, \
            "no record for task id %lu", task_id);
    }

    pthread_mutex_unlock(&thread_mutex);
}

static void
test(unsigned int nested, unsigned int outer_threads, unsigned int inner_threads)
{
    unsigned int i;

#if DEBUG
    printf("%s: nested=%d, outer_threads=%d, inner_threads=%d\n",
        __func__, nested, outer_threads, inner_threads);
#endif

    omp_set_nested(nested);

    #pragma omp parallel num_threads(outer_threads)
    {
        // Force library call for gcc; not done with schedule(static)
        #pragma omp parallel for schedule(runtime) num_threads(inner_threads)
        for (i = 0; i < NUM_THREADS; i++) {
            serialwork(0);
        }
    }

    #pragma omp parallel num_threads(outer_threads)
    {
        #pragma omp parallel num_threads(inner_threads)
        {
            #pragma omp for schedule(runtime)
            for (i = 0; i < NUM_THREADS; i++) {
                serialwork(0);
            }

            serialwork(0);

            #pragma omp for schedule(runtime)
            for (i = 0; i < NUM_THREADS; i++) {
                serialwork(0);
            }
        }
    }
}

static void
check_balanced_calls(void)
{
    CHECK(loop_begin == loop_end, IMPLEMENTED_BUT_INCORRECT, \
        "unbalanced number of calls to begin and end callbacks (%d vs %d)", \
        loop_begin, loop_end);
}

//*****************************************************************************
// interface operations 
//*****************************************************************************

void 
init_test(ompt_function_lookup_t lookup)
{
    if (!register_callback(ompt_event_loop_begin,
        (ompt_callback_t) on_ompt_event_loop_begin)) {
        CHECK(FALSE, NOT_IMPLEMENTED, "failed to register ompt_event_loop_begin");
    }
    if (!register_callback(ompt_event_loop_end,
        (ompt_callback_t) on_ompt_event_loop_end)) {
        CHECK(FALSE, NOT_IMPLEMENTED, "failed to register ompt_event_loop_begin");
    }
}

int
regression_test(int argc, char** argv)
{
    unsigned int i, nested, outer_threads, inner_threads;

    omp_set_schedule(omp_sched_static, 0);

    // no loop events should occur
    #pragma omp parallel num_threads(NUM_THREADS)
    {
        serialwork(0);
    }
    CHECK(loop_begin == 0 && loop_end == 0, IMPLEMENTED_BUT_INCORRECT, \
        "loop events in simple parallel region (%d begin, %d ends)", \
        loop_begin, loop_end);

    loop_begin = loop_end = 0;

    // Special test for gcc
    #pragma omp parallel for schedule(static) num_threads(NUM_THREADS)
    for (i = 0; i < NUM_THREADS; i++) {
        serialwork(0);
    }
    CHECK(loop_begin != 0 && loop_end != 0, CORRECT, \
        "no loop events in 'parallel for schedule(static)'; known fail for gcc!");
    check_balanced_calls();

    loop_begin = loop_end = 0;

    // Force library call for gcc; not done with schedule(static)
    #pragma omp parallel for schedule(runtime) num_threads(NUM_THREADS)
    for (i = 0; i < NUM_THREADS; i++) {
        serialwork(0);
    }
    CHECK(loop_begin != 0 && loop_end != 0, IMPLEMENTED_BUT_INCORRECT, \
        "no loop events in 'parallel for schedule(runtime)'");
    check_balanced_calls();

    // test all combinations of nested regions
    for (nested = 0; nested <= 1; nested++) {
        for (inner_threads = 1; inner_threads <= NUM_THREADS; inner_threads++) {
            for (outer_threads = 1; outer_threads <= NUM_THREADS; outer_threads++) {
                loop_begin = loop_end = 0;

                test(nested, inner_threads, outer_threads);

                check_balanced_calls();
            }
        }
    }

    return return_code;
}
