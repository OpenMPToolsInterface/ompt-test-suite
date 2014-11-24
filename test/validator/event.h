#include <iostream>
#include <string>
#include <stdint.h>
#include <map>

#ifndef EVENT_H
#define EVENT_H

using namespace std;

typedef int64_t ompt_thread_id_t;
typedef enum ompt_thread_type_e {
	ompt_thread_initial = 1,
	ompt_thread_worker = 2,
	ompt_thread_other = 3
} ompt_thread_type_t;
typedef int64_t ompt_parallel_id_t;
typedef int64_t ompt_task_id_t;
typedef int64_t ompt_wait_id_t;
typedef struct ompt_frame_s {
	void *exit_runtime_frame;
	void *reenter_runtime_frame;
} ompt_frame_t;
// OMPT 4.0
typedef int64_t ompt_target_id_t;
typedef int64_t ompt_target_device_id_t;
typedef int64_t ompt_data_map_id_t;
typedef int64_t ompt_data_size_t;


enum EventType {
	// Mandatory Events
	ompt_event_parallel_end,
	ompt_event_parallel_begin,
	ompt_event_task_begin,
	ompt_event_task_end,
	ompt_event_thread_begin,
	ompt_event_thread_end,
	ompt_event_control,
	ompt_event_runtime_shutdown,
	// Optional Events
	ompt_event_idle_begin,
	ompt_event_idle_end,
	ompt_event_wait_barrier_begin,
	ompt_event_wait_barrier_end,
	ompt_event_wait_taskwait_begin,
	ompt_event_wait_taskwait_end,
	ompt_event_wait_taskgroup_begin,
	ompt_event_wait_taskgroup_end,
	ompt_event_release_lock,
	ompt_event_release_nest_lock_last,
	ompt_event_release_critical,
	ompt_event_release_atomic,
	ompt_event_release_ordered,
	ompt_event_implicit_task_begin,
	ompt_event_implicit_task_end,
	ompt_event_task_switch,
	ompt_event_loop_begin,
	ompt_event_loop_end,
	ompt_event_sections_begin,
	ompt_event_sections_end,
	ompt_event_single_in_block_begin,
	ompt_event_single_in_block_end,
	ompt_event_single_others_begin,
	ompt_event_single_others_end,
	ompt_event_workshare_begin,
	ompt_event_workshare_end,
	ompt_event_master_begin,
	ompt_event_master_end,
	ompt_event_barrier_begin,
	ompt_event_barrier_end,
	ompt_event_taskwait_begin,
	ompt_event_taskwait_end,
	ompt_event_taskgroup_begin,
	ompt_event_taskgroup_end,
	ompt_event_release_nest_lock_prev,
	ompt_event_wait_lock,
	ompt_event_wait_nest_lock,
	ompt_event_wait_critical,
	ompt_event_wait_ordered,
	ompt_event_acquired_lock,
	ompt_event_acquired_nest_lock_first,
	ompt_event_acquired_nest_lock_next,
	ompt_event_acquired_critical,
	ompt_event_acquired_atomic,
	ompt_event_acquired_ordered,
	ompt_event_init_lock,
	ompt_event_init_nest_lock,
	ompt_event_destroy_lock,
	ompt_event_destroy_nest_lock,
	ompt_event_flush,
	// OMPT 4.0 Target Events
	ompt_event_target_begin,
	ompt_event_target_end,
	ompt_event_target_data_begin,
	ompt_event_target_data_end,
	ompt_event_target_update_begin,
	ompt_event_target_update_end,
	ompt_event_data_map_begin,
	ompt_event_data_map_end,
	ompt_event_target_invoke_begin,
	ompt_event_target_invoke_end
};

// map string values to enum values
inline map<string, EventType> create_map() {
	map<string, EventType> m;
	// Mandatory Events
	m["ompt_event_parallel_begin"] = ompt_event_parallel_begin;
	m["ompt_event_parallel_end"] = ompt_event_parallel_end;
	m["ompt_event_task_begin"] = ompt_event_task_begin;
	m["ompt_event_task_end"] = ompt_event_task_end;
	m["ompt_event_thread_begin"] = ompt_event_thread_begin;
	m["ompt_event_thread_end"] = ompt_event_thread_end;
	m["ompt_event_control"] = ompt_event_control;
	m["ompt_event_runtime_shutdown"] = ompt_event_runtime_shutdown;
	// Optional Events
	m["ompt_event_idle_begin"] = ompt_event_idle_begin;
	m["ompt_event_idle_end"] = ompt_event_idle_end;
	m["ompt_event_wait_barrier_begin"] = ompt_event_wait_barrier_begin;
	m["ompt_event_wait_barrier_end"] = ompt_event_wait_barrier_end;
	m["ompt_event_wait_taskwait_begin"] = ompt_event_wait_taskwait_begin;
	m["ompt_event_wait_taskwait_end"] = ompt_event_wait_taskwait_end;
	m["ompt_event_wait_taskgroup_begin"] = ompt_event_wait_taskgroup_begin;
	m["ompt_event_wait_taskgroup_end"] = ompt_event_wait_taskgroup_end;
	m["ompt_event_release_lock"] = ompt_event_release_lock;
	m["ompt_event_release_nest_lock_last"] = ompt_event_release_nest_lock_last;
	m["ompt_event_release_critical"] = ompt_event_release_critical;
	m["ompt_event_release_atomic"] = ompt_event_release_atomic;
	m["ompt_event_release_ordered"] = ompt_event_release_ordered; 
	m["ompt_event_implicit_task_begin"] = ompt_event_implicit_task_begin; 
	m["ompt_event_implicit_task_end"] = ompt_event_implicit_task_end;
	m["ompt_event_task_switch"] = ompt_event_task_switch;
	m["ompt_event_loop_begin"] = ompt_event_loop_begin;
	m["ompt_event_loop_end"] = ompt_event_loop_end;
	m["ompt_event_sections_begin"] = ompt_event_sections_begin;
	m["ompt_event_sections_end"] = ompt_event_sections_end;
	m["ompt_event_single_in_block_begin"] = ompt_event_single_in_block_begin;
	m["ompt_event_single_in_block_end"] = ompt_event_single_in_block_end;
	m["ompt_event_single_others_begin"] = ompt_event_single_others_begin; 
	m["ompt_event_single_others_end"] = ompt_event_single_others_end;
	m["ompt_event_workshare_begin"] = ompt_event_workshare_begin; 
	m["ompt_event_workshare_end"] = ompt_event_workshare_end;
	m["ompt_event_master_begin"] = ompt_event_master_begin;	
	m["ompt_event_master_end"] = ompt_event_master_end;
	m["ompt_event_barrier_begin"] = ompt_event_barrier_begin;
	m["ompt_event_barrier_end"] = ompt_event_barrier_end;
	m["ompt_event_taskwait_begin"] = ompt_event_taskwait_begin;
	m["ompt_event_taskwait_end"] = ompt_event_taskwait_end;
	m["ompt_event_taskgroup_begin"] = ompt_event_taskgroup_begin;
	m["ompt_event_taskgroup_end"] = ompt_event_taskgroup_end;
	m["ompt_event_release_nest_lock_prev"] = ompt_event_release_nest_lock_prev;
	m["ompt_event_wait_lock"] = ompt_event_wait_lock;
	m["ompt_event_wait_nest_lock"] = ompt_event_wait_nest_lock;
	m["ompt_event_wait_critical"] = ompt_event_wait_critical;
	m["ompt_event_wait_ordered"] = ompt_event_wait_ordered;
	m["ompt_event_acquired_lock"] = ompt_event_acquired_lock;
	m["ompt_event_acquired_nest_lock_first"] = ompt_event_acquired_nest_lock_first;
	m["ompt_event_acquired_nest_lock_next"] = ompt_event_acquired_nest_lock_next;
	m["ompt_event_acquired_critical"] = ompt_event_acquired_critical;
	m["ompt_event_acquired_atomic"] = ompt_event_acquired_atomic;
	m["ompt_event_acquired_ordered"] = ompt_event_acquired_ordered;
	m["ompt_event_init_lock"] = ompt_event_init_lock;
	m["ompt_event_init_nest_lock"] = ompt_event_init_nest_lock;
	m["ompt_event_destroy_lock"] = ompt_event_destroy_lock;
	m["ompt_event_destroy_nest_lock"] = ompt_event_destroy_nest_lock;
	m["ompt_event_flush"] = ompt_event_flush;
	// OMPT 4.0 Target Events
	m["ompt_event_target_begin"] = ompt_event_target_begin;
	m["ompt_event_target_end"] = ompt_event_target_end;
	m["ompt_event_target_data_begin"] = ompt_event_target_data_begin;
	m["ompt_event_target_data_end"] = ompt_event_target_data_end;
	m["ompt_event_target_update_begin"] = ompt_event_target_update_begin;
	m["ompt_event_target_update_end"] = ompt_event_target_update_end;
	m["ompt_event_data_map_begin"] = ompt_event_data_map_begin;
	m["ompt_event_data_map_end"] = ompt_event_data_map_end;
	m["ompt_event_target_invoke_begin"] = ompt_event_target_invoke_begin;
	m["ompt_event_target_invoke_end"] = ompt_event_target_invoke_end;

	return m;
};

static map<string, EventType> eventtype_map = create_map();


// Define structs for different types of signatures

// Base struct for all event signatures
struct ompt_event_s {
	EventType type;
	string name;

	virtual ompt_task_id_t get_task_id() { return -1; }
};

// Mandatory Events

// ompt_thread_type_callback_t
struct ompt_thread_type_s : ompt_event_s {
	ompt_thread_type_t thread_type;
	ompt_thread_id_t thread_id;
};

// ompt_parallel_callback_t
struct ompt_parallel_s : ompt_event_s {
	ompt_parallel_id_t parallel_id;
	ompt_task_id_t task_id;

	ompt_task_id_t get_task_id() { return task_id; }
};

// ompt_new_parallel_callback_t
struct ompt_new_parallel_s : ompt_event_s {
	ompt_task_id_t parent_task_id;
	ompt_frame_t *parent_task_frame;
	ompt_parallel_id_t parallel_id;
	uint32_t requested_team_size;
	void *parallel_function;

	// TODO: What should return get_task_id() here?
	ompt_task_id_t get_task_id() { return parent_task_id; }
};

// ompt_task_callback_t
struct ompt_task_s : ompt_event_s {
	ompt_task_id_t task_id;
};

// ompt_new_task_callback_t
struct ompt_new_task_s : ompt_event_s {
	ompt_task_id_t parent_task_id;
	ompt_frame_t *parent_task_frame;
	ompt_task_id_t new_task_id;
	void *new_task_function;

	ompt_task_id_t get_task_id() { return new_task_id; }
};

// ompt_control_callback_t
struct ompt_control_s : ompt_event_s {
	uint64_t command;
	uint64_t modifier;
};

// ompt_callback_t
struct ompt_callback_s : ompt_event_s {

};

// OMPT 4.0 Target Events

// ompt_target_callback_t
struct ompt_target_s : ompt_event_s {
	ompt_task_id_t task_id;
	ompt_target_id_t target_id;

	ompt_task_id_t get_task_id() { return task_id; }
};

// ompt_new_target_callback_t
struct ompt_new_target_s : ompt_event_s {
	ompt_task_id_t task_id;
	ompt_target_id_t target_id;
	ompt_target_device_id_t device_id;
	
	ompt_task_id_t get_task_id() { return task_id; }
};

// ompt_data_map_t
struct ompt_data_map_s : ompt_event_s {
	ompt_task_id_t task_id;
	ompt_target_id_t target_id;
	ompt_data_map_id_t data_map_id;

	ompt_task_id_t get_task_id() { return task_id; }
};

// ompt_new_data_map_t
struct ompt_new_data_map_s : ompt_event_s {
	ompt_task_id_t task_id;
	ompt_target_id_t target_id;
	ompt_data_map_id_t data_map_id;
	ompt_target_device_id_t device_id;

	ompt_task_id_t get_task_id() { return task_id; }
};

#endif
