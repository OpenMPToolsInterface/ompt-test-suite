#include <vector>

#include "event.h"

using namespace std;

#ifndef EVENTLIST_H
#define EVENTLIST_H

/**
 * Data structure to store all kind of events
 */

class EventList {
	private:
		// list containing all added events
		vector<ompt_event_s*> events;
	public:
		vector<ompt_event_s*> getAllEvents();	

		/**
 		 * add arbitrary event to list
 		 */
		void add(ompt_event_s* event);
		
		/** 
		 * get all events of 'type' with signature T in a list
		 */
		template <typename T>
		vector<T> getEvents(EventType type) {
			vector<T> list;
			for (unsigned int i=0; i < events.size(); i++) {
				if (events.at(i)->type == type) {
					list.push_back(dynamic_cast<T>(events.at(i)));
				}
			}	

			return list;
		}
};

#endif
