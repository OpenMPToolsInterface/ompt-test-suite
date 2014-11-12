#ifndef OMPT_NO_INSTROPECTION
/* 
 * This header file implements a dummy tool which will execute all
 * of the implemented callbacks in the OMPT framework. When a supported
 * callback function is executed, it will print a message with some
 * relevant information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ompt.h>

/*
 * Macros to help generate test functions for each event
 */

#define TEST_THREAD_CALLBACK(EVENT) \
void my_##EVENT(ompt_thread_id_t thread_id) \
{ \
  printf("%d: %s: thread_id=%lu\n", omp_get_thread_num(), #EVENT, thread_id); \
  fflush(stdout); \
}

#define TEST_THREAD_TYPE_CALLBACK(EVENT) \
void my_##EVENT(ompt_thread_type_t thread_type, ompt_thread_id_t thread_id) \
{ \
  const char * type_strings[] = {"initial", "worker", "other"}; \
  printf("%d: %s: thread_id=%lu thread_type=%d type_string='%s'\n", omp_get_thread_num(), #EVENT, thread_id, thread_type, type_strings[thread_type-1]); \
  fflush(stdout); \
}

#define TEST_WAIT_CALLBACK(EVENT) \
void my_##EVENT ( \
  ompt_wait_id_t waitid)            /* address of wait obj */ \
{ \
  printf("%d: %s: waid_id=%lu\n", omp_get_thread_num(), #EVENT, waitid); \
  fflush(stdout); \
}

#define TEST_PARALLEL_CALLBACK(EVENT) \
void my_##EVENT ( \
ompt_parallel_id_t parallel_id,   /* id of parallel region       */ \
ompt_task_id_t task_id)           /* id for task                 */ \
{ \
  printf("%d: %s: parallel_id=%lu task_id=%lu\n", omp_get_thread_num(), #EVENT, parallel_id, task_id); \
  fflush(stdout); \
}

#define TEST_NEW_WORKSHARE_CALLBACK(EVENT) \
void my_##EVENT ( \
ompt_parallel_id_t parallel_id,   /* id of parallel region       */ \
ompt_task_id_t task_id,           /* id for task                 */ \
void *workshare_function)           /* ptr to outlined function  */ \
{ \
  printf("%d: %s: parallel_id=%lu task_id=%lu workshare_function=%p\n", omp_get_thread_num(), #EVENT, parallel_id, task_id, workshare_function); \
  fflush(stdout); \
}

#define TEST_NEW_PARALLEL_CALLBACK(EVENT) \
void my_##EVENT ( \
  ompt_task_id_t  parent_task_id,   /* tool data for parent task    */ \
  ompt_frame_t *parent_task_frame,  /* frame data of parent task    */ \
  ompt_parallel_id_t parallel_id,   /* id of parallel region        */ \
  uint32_t requested_team_size,     /* # threads requested for team */ \
  void *parallel_function)          /* outlined function            */ \
{ \
  printf("%d: %s: parent_task_id=%lu parent_task_frame=%p parallel_id=%lu team_size=%lu parallel_function=%p\n", omp_get_thread_num(), #EVENT, parent_task_id, parent_task_frame, parallel_id, parent_task_id, parallel_function); \
  fflush(stdout); \
}

#define TEST_TASK_CALLBACK(EVENT) \
void my_##EVENT ( \
ompt_task_id_t task_id)            /* tool data for task          */ \
{ \
  printf("%d: %s: task_id=%lu\n", omp_get_thread_num(), #EVENT, task_id); \
  fflush(stdout); \
} \

#define TEST_TASK_SWITCH_CALLBACK(EVENT) \
void my_##EVENT ( \
  ompt_task_id_t suspended_task_id, /* tool data for suspended task */ \
  ompt_task_id_t resumed_task_id)   /* tool data for resumed task   */ \
{ \
  printf("%d: %s: suspended_task_id=%lu resumed_task_id=%lu\n", omp_get_thread_num(), #EVENT, suspended_task_id, resumed_task_id); \
  fflush(stdout); \
}

#define TEST_NEW_TASK_CALLBACK(EVENT) \
void my_##EVENT ( \
  ompt_task_id_t  parent_task_id,   /* tool data for parent task   */ \
  ompt_frame_t *parent_task_frame,  /* frame data of parent task   */ \
  ompt_task_id_t new_task_id,   /* id of parallel region       */ \
  void *new_task_function)          /* outlined function           */ \
{ \
  printf("%d: %s: parent_task_id=%lu parent_task_frame-=%p new_task_id=%lu parallel_function=%p\n", omp_get_thread_num(), #EVENT, parent_task_id, parent_task_frame, new_task_id, new_task_function); \
  fflush(stdout); \
}

#define TEST_CONTROL_CALLBACK(EVENT) \
void my_##EVENT( \
uint64_t command,                /* command of control call      */ \
uint64_t modifier)                /* modifier of control call     */ \
{ \
  printf("%d: %s: command=%lu modifier=%lu\n", omp_get_thread_num(), #EVENT, command, modifier); \
  fflush(stdout); \
}


#define TEST_CALLBACK(EVENT) \
void my_##EVENT() \
{ \
  printf("%d: %s\n", omp_get_thread_num(), #EVENT); \
  fflush(stdout); \
}

#define TEST_NEW_TARGET_CALLBACK(event) \
void my_##event( \
  ompt_task_id_t task_id,            /* ID of task */ \
  ompt_target_id_t target_id,        /* ID of target* region */ \
  ompt_target_device_id_t device_id, /* ID of the device */ \
  void *target_function              /* pointer to outlined function */ \
    ) \
{ \
  printf("%d: %s: task_id=%llu target_id=%llu device_id=%llu target_function=%llu\n", omp_get_thread_num(), #event, task_id, target_id, device_id, target_function); \
  fflush(0); \
} 


#define TEST_TARGET_CALLBACK(event) \
void my_##event( \
  ompt_task_id_t task_id,            /* ID of task */ \
  ompt_target_id_t target_id         /* ID of target* region */ \
    ) \
{ \
  printf("%d: %s: task_id=%llu target_id=%llu\n", omp_get_thread_num(), #event, task_id, target_id); \
  fflush(0); \
}

#define TEST_NEW_DATA_MAP_CALLBACK(event) \
void my_##event( \
  ompt_task_id_t task_id,            /* ID of task */ \
  ompt_target_id_t target_id,        /* ID of target* region */ \
  ompt_data_map_id_t data_map_id,    /* ID of data map operation */ \
  ompt_target_device_id_t device_id, /* ID of the device */ \
  ompt_target_sync_t sync_type,      /* synchronous or asynchronus data mapping */ \
  ompt_data_map_t map_type,          /* type of the data mapping / motion */ \
  uint64_t bytes                     /* amount of mapped bytes */ \
    ) \
{ \
  printf("%d: %s: task_id=%llu target_id=%llu data_map_id=%llu device_id=%llu sync_type=%s map_type=%s bytes=%llu\n", \
     omp_get_thread_num(), \
     #event, \
     task_id, target_id, \
     data_map_id, device_id, \
     sync_type == ompt_data_sync ? "SYNC" : "ASYNC", \
     map_type == ompt_data_map_IN ? "IN" : "OUT", \
     bytes); \
  fflush(0); \
} 


#define TEST_DATA_MAP_CALLBACK(event) \
void my_##event( \
  ompt_task_id_t task_id,           /* ID of task */ \
  ompt_target_id_t target_id,       /* ID of target* region */ \
  ompt_data_map_id_t data_map_id    /* ID of data map operation */ \
    ) \
{ \
  printf("%d: %s: task_id=%llu target_id=%llu data_map_id=%llu\n", \
     omp_get_thread_num(), \
     #event, \
     task_id, target_id, data_map_id); \
  fflush(0); \
} 


/*******************************************************************
 * Function declaration
 *******************************************************************/

#define OMPT_FN_TYPE(fn) fn ## _t 
#define OMPT_FN_LOOKUP(lookup,fn) fn = (OMPT_FN_TYPE(fn)) lookup(#fn)
#define OMPT_FN_DECL(fn) OMPT_FN_TYPE(fn) fn

OMPT_FN_DECL(ompt_get_task_frame);
OMPT_FN_DECL(ompt_set_callback);
OMPT_FN_DECL(ompt_get_task_id);
OMPT_FN_DECL(ompt_get_parallel_id);
OMPT_FN_DECL(ompt_get_thread_id);

/*******************************************************************
 * required events 
 *******************************************************************/

TEST_THREAD_TYPE_CALLBACK(ompt_event_thread_begin)
TEST_THREAD_TYPE_CALLBACK(ompt_event_thread_end)
TEST_NEW_PARALLEL_CALLBACK(ompt_event_parallel_begin)
TEST_PARALLEL_CALLBACK(ompt_event_parallel_end)
TEST_NEW_TASK_CALLBACK(ompt_event_task_begin)
TEST_NEW_TASK_CALLBACK(ompt_event_task_end)
TEST_CONTROL_CALLBACK(ompt_event_control)
TEST_CALLBACK(ompt_event_runtime_shutdown)

/*******************************************************************
 * optional events
 *******************************************************************/

/* Blameshifting events */
TEST_THREAD_CALLBACK(ompt_event_idle_begin)
TEST_THREAD_CALLBACK(ompt_event_idle_end)
TEST_PARALLEL_CALLBACK(ompt_event_wait_barrier_begin)
TEST_PARALLEL_CALLBACK(ompt_event_wait_barrier_end)
TEST_PARALLEL_CALLBACK(ompt_event_wait_taskwait_begin)
TEST_PARALLEL_CALLBACK(ompt_event_wait_taskwait_end)
TEST_PARALLEL_CALLBACK(ompt_event_wait_taskgroup_begin)
TEST_PARALLEL_CALLBACK(ompt_event_wait_taskgroup_end)
TEST_WAIT_CALLBACK(ompt_event_release_lock)
TEST_WAIT_CALLBACK(ompt_event_release_nest_lock_last)
TEST_WAIT_CALLBACK(ompt_event_release_critical)
TEST_WAIT_CALLBACK(ompt_event_release_ordered)
TEST_WAIT_CALLBACK(ompt_event_release_atomic)

/* synchronous events */
TEST_PARALLEL_CALLBACK(ompt_event_implicit_task_begin)
TEST_PARALLEL_CALLBACK(ompt_event_implicit_task_end)
TEST_PARALLEL_CALLBACK(ompt_event_initial_task_begin)
TEST_PARALLEL_CALLBACK(ompt_event_initial_task_end)
TEST_TASK_SWITCH_CALLBACK(ompt_event_task_switch)
TEST_WAIT_CALLBACK(ompt_event_init_lock)
TEST_WAIT_CALLBACK(ompt_event_init_nest_lock)
TEST_WAIT_CALLBACK(ompt_event_destroy_lock)
TEST_WAIT_CALLBACK(ompt_event_destroy_nest_lock)
TEST_NEW_WORKSHARE_CALLBACK(ompt_event_loop_begin)
TEST_PARALLEL_CALLBACK(ompt_event_loop_end)
TEST_NEW_WORKSHARE_CALLBACK(ompt_event_sections_begin)
TEST_PARALLEL_CALLBACK(ompt_event_sections_end)
TEST_NEW_WORKSHARE_CALLBACK(ompt_event_single_in_block_begin)
TEST_PARALLEL_CALLBACK(ompt_event_single_in_block_end)
TEST_PARALLEL_CALLBACK(ompt_event_single_others_begin)
TEST_PARALLEL_CALLBACK(ompt_event_single_others_end)
TEST_NEW_WORKSHARE_CALLBACK(ompt_event_workshare_begin)
TEST_PARALLEL_CALLBACK(ompt_event_workshare_end)
TEST_PARALLEL_CALLBACK(ompt_event_master_begin)
TEST_PARALLEL_CALLBACK(ompt_event_master_end)
TEST_PARALLEL_CALLBACK(ompt_event_barrier_begin)
TEST_PARALLEL_CALLBACK(ompt_event_barrier_end)
TEST_PARALLEL_CALLBACK(ompt_event_taskwait_begin)
TEST_PARALLEL_CALLBACK(ompt_event_taskwait_end)
TEST_PARALLEL_CALLBACK(ompt_event_taskgroup_begin)
TEST_PARALLEL_CALLBACK(ompt_event_taskgroup_end)
TEST_WAIT_CALLBACK(ompt_event_wait_lock)
TEST_WAIT_CALLBACK(ompt_event_acquired_lock)
TEST_WAIT_CALLBACK(ompt_event_wait_nest_lock)
TEST_WAIT_CALLBACK(ompt_event_acquired_nest_lock_first)
TEST_PARALLEL_CALLBACK(ompt_event_release_nest_lock_prev)
TEST_PARALLEL_CALLBACK(ompt_event_acquired_nest_lock_next)
TEST_WAIT_CALLBACK(ompt_event_wait_critical)
TEST_WAIT_CALLBACK(ompt_event_acquired_critical)
TEST_WAIT_CALLBACK(ompt_event_wait_ordered)
TEST_WAIT_CALLBACK(ompt_event_acquired_ordered)
TEST_WAIT_CALLBACK(ompt_event_wait_atomic)
TEST_WAIT_CALLBACK(ompt_event_acquired_atomic)
TEST_THREAD_CALLBACK(ompt_event_flush)

#ifdef OMPT_TARGET
/*******************************************************************
 * target events (not yet in technical report)
 *******************************************************************/
TEST_NEW_TARGET_CALLBACK(ompt_event_target_begin)
TEST_TARGET_CALLBACK(ompt_event_target_end)

TEST_NEW_TARGET_CALLBACK(ompt_event_target_data_begin)
TEST_TARGET_CALLBACK(ompt_event_target_data_end)

TEST_NEW_DATA_MAP_CALLBACK(ompt_event_data_map_begin)
TEST_DATA_MAP_CALLBACK(ompt_event_data_map_end)

TEST_NEW_TARGET_CALLBACK(ompt_event_target_update_begin)
TEST_TARGET_CALLBACK(ompt_event_target_update_end)

TEST_NEW_TARGET_CALLBACK(ompt_event_target_invoke_begin)
TEST_TARGET_CALLBACK(ompt_event_target_invoke_end)
#endif


/*******************************************************************
 * Register the events
 *******************************************************************/

#define OMPT_CHECK_VERBOSE

#ifdef OMPT_CHECK_VERBOSE
#define CHECK(EVENT) \
if (ompt_set_callback(EVENT, (ompt_callback_t) my_##EVENT) == 0) { \
  fprintf(stderr,"Failed to register OMPT callback %s (not implemented)\n",#EVENT);  \
}
#else
#define CHECK(EVENT) ompt_set_callback(EVENT, (ompt_callback_t) my_##EVENT); 
#endif


int ompt_initialize(ompt_function_lookup_t lookup, const char *runtime_version, int ompt_version) {
  printf("Init: %s ver %i\n",runtime_version,ompt_version);

  /* look up and bind OMPT API functions */

  OMPT_FN_LOOKUP(lookup,ompt_set_callback);
  OMPT_FN_LOOKUP(lookup,ompt_get_task_frame);
  OMPT_FN_LOOKUP(lookup,ompt_get_task_id);
  OMPT_FN_LOOKUP(lookup,ompt_get_parallel_id);
  OMPT_FN_LOOKUP(lookup,ompt_get_thread_id);

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

  CHECK(ompt_event_idle_begin);
  CHECK(ompt_event_idle_end);
  CHECK(ompt_event_wait_barrier_begin);
  CHECK(ompt_event_wait_barrier_end);
  CHECK(ompt_event_wait_taskwait_begin);
  CHECK(ompt_event_wait_taskwait_end);
  CHECK(ompt_event_wait_taskgroup_begin);
  CHECK(ompt_event_wait_taskgroup_end);
  CHECK(ompt_event_release_lock);
  CHECK(ompt_event_release_nest_lock_last);
  CHECK(ompt_event_release_critical);
  CHECK(ompt_event_release_atomic);
  CHECK(ompt_event_release_ordered);

  /* optional events, synchronous */

  CHECK(ompt_event_implicit_task_begin);
  CHECK(ompt_event_implicit_task_end);
  CHECK(ompt_event_master_begin);
  CHECK(ompt_event_master_end);
  CHECK(ompt_event_barrier_begin);
  CHECK(ompt_event_barrier_end);
  CHECK(ompt_event_task_switch);
  CHECK(ompt_event_loop_begin);
  CHECK(ompt_event_loop_end);
  CHECK(ompt_event_sections_begin);
  CHECK(ompt_event_sections_end);
  CHECK(ompt_event_single_in_block_begin);
  CHECK(ompt_event_single_in_block_end);
  CHECK(ompt_event_single_others_begin);
  CHECK(ompt_event_single_others_end);
  CHECK(ompt_event_taskwait_begin);
  CHECK(ompt_event_taskwait_end);
  CHECK(ompt_event_taskgroup_begin);
  CHECK(ompt_event_taskgroup_end);
  CHECK(ompt_event_release_nest_lock_prev);
  CHECK(ompt_event_wait_lock);
  CHECK(ompt_event_wait_nest_lock);
  CHECK(ompt_event_wait_critical);
  CHECK(ompt_event_wait_atomic);
  CHECK(ompt_event_wait_ordered);
  CHECK(ompt_event_acquired_lock);
  CHECK(ompt_event_acquired_nest_lock_first);
  CHECK(ompt_event_acquired_nest_lock_next);
  CHECK(ompt_event_acquired_critical);
  CHECK(ompt_event_acquired_atomic);
  CHECK(ompt_event_acquired_ordered);
  CHECK(ompt_event_init_lock);
  CHECK(ompt_event_init_nest_lock);
  CHECK(ompt_event_destroy_lock);
  CHECK(ompt_event_destroy_nest_lock);
  CHECK(ompt_event_flush);
  
#ifdef OMPT_TARGET
  /* targt* events */
  CHECK(ompt_event_target_begin);
  CHECK(ompt_event_target_end);
  CHECK(ompt_event_target_data_begin);
  CHECK(ompt_event_target_data_end);
  CHECK(ompt_event_data_map_begin);
  CHECK(ompt_event_data_map_end);
  CHECK(ompt_event_target_update_begin);
  CHECK(ompt_event_target_update_end);
  CHECK(ompt_event_target_invoke_begin);
  CHECK(ompt_event_target_invoke_end);
#endif

  return 1;
}

#endif
