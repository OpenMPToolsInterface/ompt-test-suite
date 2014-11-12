#include <iostream>
#include <vector>

#include "event.h"
#include "eventlist.h"

#ifndef TESTS_H_
#define TESTS_H_

/**
 * A class containing different types of tests.
 */
class Tests {
	public:
		/**
 		 * Check if there is a parallel end for each parallel begin
 		 */
		static bool check_parallel_begin_end(EventList list);

		/**
 		 * Check if there is a target end for each target begin
 		 */
		static bool check_target_begin_end(EventList list);

		/**
 		 * Check if there is a data map end for each data map begin
 		 */
		static bool check_data_map_begin_end(EventList list);

		/**
		 * Check if all task_ids are not 0
		 */
		static bool check_task_ids(EventList e);
};

#endif
