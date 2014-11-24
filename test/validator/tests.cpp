#include "tests.h"

bool Tests::check_parallel_begin_end(EventList list) {
	// retrieve all parallel begin / end events in separate lists
	vector<ompt_new_parallel_s*> beginlist = 
		list.getEvents<ompt_new_parallel_s*>(ompt_event_parallel_begin);
	vector<ompt_parallel_s*> endlist = 
		list.getEvents<ompt_parallel_s*>(ompt_event_parallel_end);

	ompt_new_parallel_s* beginitem;
	ompt_parallel_s* enditem;

	bool result = true;

	// for each begin item try to find an end item with the same parallel_id	
	for (unsigned int i=0; i < beginlist.size(); i++) {
		beginitem = beginlist.at(i);

		bool found = false;
		for (unsigned int j=0; j < endlist.size(); j++) {
			enditem = endlist.at(j);

			if (beginitem->parallel_id == enditem->parallel_id) {
				found = true;
				break;
			}
		}

		if (!found) {
			cout << "ERROR: No parallel_end found for event parallel_begin (parallel_id="
				<< beginitem->parallel_id  << ")" << endl;
			result = false;
		}
	}

	return result;
}

bool Tests::check_target_begin_end(EventList list) {
	// retrieve all target begin / end events in separate lists
	vector<ompt_new_target_s*> beginlist = 
		list.getEvents<ompt_new_target_s*>(ompt_event_target_begin);
	vector<ompt_target_s*> endlist = 
		list.getEvents<ompt_target_s*>(ompt_event_target_end);

	ompt_new_target_s* beginitem;
	ompt_target_s* enditem;

	bool result = true;

	// for each begin item try to find an end item with the same target_id	
	for (unsigned int i=0; i < beginlist.size(); i++) {
		beginitem = beginlist.at(i);

		bool found = false;
		for (unsigned int j=0; j < endlist.size(); j++) {
			enditem = endlist.at(j);

			if (beginitem->target_id == enditem->target_id) {
				found = true;
				break;
			}
		}

		if (!found) {
			cout << "ERROR: No target_end found for event target_begin (task_id="
				<< beginitem->task_id  << ", target_id="
				<< beginitem->target_id << ")" << endl;
			result = false;
		}
	}

	return result;
}

bool Tests::check_data_map_begin_end(EventList list) {
	// retrieve all data map begin / end events in separate lists
	vector<ompt_new_data_map_s*> beginlist =
		list.getEvents<ompt_new_data_map_s*>(ompt_event_data_map_begin);
	vector<ompt_data_map_s*> endlist = 
		list.getEvents<ompt_data_map_s*>(ompt_event_data_map_end);

	ompt_new_data_map_s* beginitem;
	ompt_data_map_s* enditem;

	bool result = true;
	
	// for each begin item try to find an end item with the same data_map_id	
	for (unsigned int i=0; i < beginlist.size(); i++) {
		beginitem = beginlist.at(i);

		bool found = false;
		for (unsigned int j=0; j < endlist.size(); j++) {
			enditem = endlist.at(j);
	
			if (beginitem->data_map_id == enditem->data_map_id) {
				found = true;
				break;
			}
		} 
		
		if (!found) {
			cout << "ERROR: No data_map_end found for event data_map_begin (data_map_id="
				<< beginitem->data_map_id << ")" << endl;
			result = false;
		}
	}

	return result;
}


bool Tests::check_task_ids(EventList list) {
	bool result = true;
	vector<ompt_event_s*> events = list.getAllEvents();

	for (unsigned int i=0; i < events.size(); i++) {
		if (events.at(i)->get_task_id() == 0) {
			result = false;
			cout << "ERROR: task_id = 0 in " << events.at(i)->name << endl;
		}
	}
	
	return result;
}
