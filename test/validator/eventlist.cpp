#include "eventlist.h"

void EventList::add(ompt_event_s* event) {
	events.push_back(event);
}


vector<ompt_event_s*> EventList::getAllEvents() {
	return this->events;
}

