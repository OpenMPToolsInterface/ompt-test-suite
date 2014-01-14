#ifndef TAU_OPENMP
/* 
 * This header file implements a dummy tool which will execute all
 * of the implemented callbacks in the OMPT framework. When a supported
 * callback function is executed, it will print a message with some
 * relevant information.
 */

#include <stdlib.h>
#include <stdio.h>
#include <ompt.h>

/*
 * Macros to help generate test functions for each event
 */

#define TEST_PARALLEL_CALLBACK(EVENT) \
void my_##EVENT ( \
ompt_parallel_id_t parallel_id,   /* id of parallel region       */ \
ompt_task_id_t task_id)           /* id for task                 */ \
{ \
  printf("%d: %s: par_id=0x%llx task_id=0x%llx\n", omp_get_thread_num(), #EVENT, parallel_id, task_id); \
  fflush(stdout); \
}

#define TEST_NEW_WORKSHARE_CALLBACK(EVENT) \
void my_##EVENT ( \
ompt_parallel_id_t parallel_id,   /* id of parallel region       */ \
ompt_task_id_t task_id,           /* id for task                 */ \
void *workshare_function)           /* ptr to outlined function  */ \
{ \
  printf("%d: %s: par_id=0x%llx task_id=0x%llx\n", omp_get_thread_num(), #EVENT, parallel_id, task_id); \
  fflush(stdout); \
}


#define TEST_NEW_PARALLEL_CALLBACK(EVENT) \
void my_##EVENT ( \
  ompt_task_id_t  parent_task_id,   /* tool data for parent task   */ \
  ompt_frame_t *parent_task_frame,  /* frame data of parent task   */ \
  ompt_parallel_id_t parallel_id,   /* id of parallel region       */ \
  void *parallel_function)          /* outlined function           */ \
{ \
  printf("%d: %s: par_id=0x%llx task_id=0x%llx par_fn=%p\n", omp_get_thread_num(), #EVENT, parallel_id, parent_task_id, parallel_function); \
  fflush(stdout); \
}

#define TEST_TASK_CALLBACK(EVENT) \
void my_##EVENT ( \
ompt_task_id_t parent_task_id, ompt_frame_t *parent_frame, ompt_task_id_t task_id)            /* tool data for task          */ \
{ \
  printf("%d: %s: task_id=0x%llx\n", omp_get_thread_num(), #EVENT, task_id); \
  fflush(stdout); \
} \

#define TEST_CONTROL_CALLBACK(EVENT) \
void my_##EVENT( \
uint64_t command,                /* command of control call      */ \
uint64_t modifier)                /* modifier of control call     */ \
{ \
  printf("%d: %s: cmd=0x%llx, mod=0x%llx\n", omp_get_thread_num(), #EVENT, command, modifier); \
  fflush(stdout); \
}

#define TEST_CALLBACK(EVENT) \
void my_##EVENT() \
{ \
  printf("%d: %s\n", omp_get_thread_num(), #EVENT); \
  fflush(stdout); \
}

#define TEST_WAIT_CALLBACK(EVENT) \
void my_##EVENT ( \
  ompt_wait_id_t waitid)            /* address of wait obj */ \
{ \
  printf("%d: %s: waid_id=0x%llx\n", omp_get_thread_num(), #EVENT, waitid); \
  fflush(stdout); \
}

#define TEST_TASK_SWITCH_CALLBACK(EVENT) \
void my_##EVENT ( \
  ompt_task_id_t suspended_task_id, /* tool data for suspended task */ \
  ompt_task_id_t resumed_task_id)   /* tool data for resumed task   */ \
{ \
  printf("%d: %s: susp_task=0x%llx res_task=0x%llx\n", omp_get_thread_num(), #EVENT, suspended_task_id, resumed_task_id); \
  fflush(stdout); \
}

/*******************************************************************
 * required events 
 *******************************************************************/
TEST_NEW_PARALLEL_CALLBACK(ompt_event_parallel_begin)
TEST_PARALLEL_CALLBACK(ompt_event_parallel_end)
TEST_TASK_CALLBACK(ompt_event_task_begin)
TEST_TASK_CALLBACK(ompt_event_task_end)
TEST_CALLBACK(ompt_event_thread_begin)
TEST_CALLBACK(ompt_event_thread_end)
TEST_CONTROL_CALLBACK(ompt_event_control)
TEST_CALLBACK(ompt_event_runtime_shutdown)

/*******************************************************************
 * optional events
 *******************************************************************/

/* Blameshifting events */
TEST_CALLBACK(ompt_event_idle_begin)
TEST_CALLBACK(ompt_event_idle_end)
TEST_PARALLEL_CALLBACK(ompt_event_wait_barrier_begin);
TEST_PARALLEL_CALLBACK(ompt_event_wait_barrier_end);
TEST_PARALLEL_CALLBACK(ompt_event_wait_taskwait_begin);
TEST_PARALLEL_CALLBACK(ompt_event_wait_taskwait_end);
TEST_PARALLEL_CALLBACK(ompt_event_wait_taskgroup_begin);
TEST_PARALLEL_CALLBACK(ompt_event_wait_taskgroup_end);
TEST_WAIT_CALLBACK(ompt_event_release_lock);
TEST_WAIT_CALLBACK(ompt_event_release_nest_lock_last);
TEST_WAIT_CALLBACK(ompt_event_release_critical);
TEST_WAIT_CALLBACK(ompt_event_release_ordered)
TEST_WAIT_CALLBACK(ompt_event_release_atomic)

/* synchronous events */
TEST_PARALLEL_CALLBACK(ompt_event_implicit_task_begin);
TEST_PARALLEL_CALLBACK(ompt_event_implicit_task_end);
TEST_PARALLEL_CALLBACK(ompt_event_barrier_begin)
TEST_PARALLEL_CALLBACK(ompt_event_barrier_end)
TEST_PARALLEL_CALLBACK(ompt_event_master_begin)
TEST_PARALLEL_CALLBACK(ompt_event_master_end)
TEST_TASK_SWITCH_CALLBACK(ompt_event_task_switch);
TEST_NEW_WORKSHARE_CALLBACK(ompt_event_loop_begin);
TEST_NEW_WORKSHARE_CALLBACK(ompt_event_loop_end);
TEST_PARALLEL_CALLBACK(ompt_event_section_begin);
TEST_PARALLEL_CALLBACK(ompt_event_section_end);
TEST_PARALLEL_CALLBACK(ompt_event_single_in_block_begin);
TEST_PARALLEL_CALLBACK(ompt_event_single_in_block_end);
TEST_PARALLEL_CALLBACK(ompt_event_single_others_begin);
TEST_PARALLEL_CALLBACK(ompt_event_single_others_end);
TEST_PARALLEL_CALLBACK(ompt_event_taskwait_begin);
TEST_PARALLEL_CALLBACK(ompt_event_taskwait_end);
TEST_PARALLEL_CALLBACK(ompt_event_taskgroup_begin);
TEST_PARALLEL_CALLBACK(ompt_event_taskgroup_end);
TEST_PARALLEL_CALLBACK(ompt_event_release_nest_lock_prev);
TEST_WAIT_CALLBACK(ompt_event_wait_lock);
TEST_WAIT_CALLBACK(ompt_event_wait_nest_lock);
TEST_WAIT_CALLBACK(ompt_event_wait_critical);
TEST_WAIT_CALLBACK(ompt_event_wait_atomic)
TEST_WAIT_CALLBACK(ompt_event_wait_ordered)
TEST_WAIT_CALLBACK(ompt_event_acquired_lock);
TEST_WAIT_CALLBACK(ompt_event_acquired_nest_lock_first);
TEST_PARALLEL_CALLBACK(ompt_event_acquired_nest_lock_next);
TEST_WAIT_CALLBACK(ompt_event_acquired_critical);
TEST_WAIT_CALLBACK(ompt_event_acquired_atomic);
TEST_WAIT_CALLBACK(ompt_event_acquired_ordered)
TEST_WAIT_CALLBACK(ompt_event_init_lock);
TEST_WAIT_CALLBACK(ompt_event_init_nest_lock);
TEST_WAIT_CALLBACK(ompt_event_destroy_lock);
TEST_WAIT_CALLBACK(ompt_event_destroy_nest_lock);
TEST_CALLBACK(ompt_event_flush);

/*******************************************************************
 * Register the events
 *******************************************************************/

#define CHECK(EVENT) \
if (ompt_set_callback(EVENT, (ompt_callback_t) my_##EVENT) == 0) { \
  fprintf(stderr,"Failed to register OMPT callback %s!\n",#EVENT);  \
}

ompt_get_task_frame_t ompt_get_task_frame;
ompt_set_callback_t ompt_set_callback;

int ompt_initialize(ompt_function_lookup_t lookup, const char *runtime_version, int ompt_version) {
  printf("Init: %s ver %i\n",runtime_version,ompt_version);
  ompt_get_task_frame = (ompt_get_task_frame_t) lookup("ompt_get_task_frame");
  ompt_set_callback = (ompt_set_callback_t) lookup("ompt_set_callback");
  /* required events */

  CHECK(ompt_event_parallel_begin);
  CHECK(ompt_event_parallel_end);
  CHECK(ompt_event_task_begin);
  CHECK(ompt_event_task_end);
  CHECK(ompt_event_thread_begin);
  CHECK(ompt_event_thread_end);
  CHECK(ompt_event_control);
  CHECK(ompt_event_runtime_shutdown);

  /* optional events, "blameshifting" */

  //CHECK(ompt_event_idle_begin);
  //CHECK(ompt_event_idle_end);
  //CHECK(ompt_event_wait_barrier_begin);
  //CHECK(ompt_event_wait_barrier_end);
  //CHECK(ompt_event_wait_taskwait_begin);
  //CHECK(ompt_event_wait_taskwait_end);
  //CHECK(ompt_event_wait_taskgroup_begin);
  //CHECK(ompt_event_wait_taskgroup_end);
  //CHECK(ompt_event_release_lock);
  //CHECK(ompt_event_release_nest_lock_last);
  //CHECK(ompt_event_release_critical);
  //CHECK(ompt_event_release_atomic);
  //CHECK(ompt_event_release_ordered);

  /* optional events, synchronous */

  //CHECK(ompt_event_implicit_task_create);
  //CHECK(ompt_event_implicit_task_exit);
  //CHECK(ompt_event_master_begin);
  //CHECK(ompt_event_master_end);
  //CHECK(ompt_event_barrier_begin);
  //CHECK(ompt_event_barrier_end);
  //CHECK(ompt_event_task_switch);
  //CHECK(ompt_event_loop_begin);
  //CHECK(ompt_event_loop_end);
  //CHECK(ompt_event_section_begin);
  //CHECK(ompt_event_section_end);
  //CHECK(ompt_event_single_in_block_begin);
  //CHECK(ompt_event_single_in_block_end);
  //CHECK(ompt_event_single_others_begin);
  //CHECK(ompt_event_single_others_end);
  //CHECK(ompt_event_taskwait_begin);
  //CHECK(ompt_event_taskwait_end);
  //CHECK(ompt_event_taskgroup_begin);
  //CHECK(ompt_event_taskgroup_end);
  //CHECK(ompt_event_release_nest_lock_prev);
  //CHECK(ompt_event_wait_lock);
  //CHECK(ompt_event_wait_nest_lock);
  //CHECK(ompt_event_wait_critical);
  //CHECK(ompt_event_wait_atomic);
  //CHECK(ompt_event_wait_ordered);
  //CHECK(ompt_event_acquired_lock);
  //CHECK(ompt_event_acquired_nest_lock_first);
  //CHECK(ompt_event_acquired_nest_lock_next);
  //CHECK(ompt_event_acquired_critical);
  //CHECK(ompt_event_acquired_atomic);
  //CHECK(ompt_event_acquired_ordered);
  //CHECK(ompt_event_init_lock);
  //CHECK(ompt_event_init_nest_lock);
  //CHECK(ompt_event_destroy_lock);
  //CHECK(ompt_event_destroy_nest_lock);
  //CHECK(ompt_event_flush);
  return 1;
}


#endif
