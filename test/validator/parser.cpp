#include "parser.h"

vector<string> Parser::explode(const string &str, char delim) {
	vector<string> tokens;
	stringstream tstream(str);
	string temp;
	
	while (getline(tstream, temp, delim)) {
		if (temp != "") {
			tokens.push_back(temp);	
		}
	}
	
	return tokens;
}


string Parser::removeSpaces(string str) {
	str.erase(remove(str.begin(), str.end(), ' '), str.end());
	
	return str;
}


void Parser::extractValue(const string &str, string &attr, string &val) {
	unsigned int equal_pos = str.find("=");
	if (equal_pos != string::npos) {
		attr.assign(str.substr(0, equal_pos));
		val  = str.substr(equal_pos+1); 
	} else {
		cout << "ERROR: Parsing error";
	}
}


void Parser::parseLine(const string &str) {
	string line(str);

	// check if this line is an event output and save position
	int event_pos = line.find("ompt_event_");
	
	// Syntax:
	// "<device_id>: <event name>: <parameter1> <parameter2> ...
	// TODO: Maybe change the syntax of the output.
	if (event_pos != string::npos) {
		vector<string> parts = explode(line, ':');
		
		// parts[0] = <device_id>:
		// parts[1] =  <event name>:
		// parts[2] = <parameter1> <parameter2>
		string event_name = removeSpaces(parts[1]);
	
		// save all found attributes in a map			
		map<string, int64_t> attributes;

		// If there are parameters
		// <parameter1> <parameter2> ...
		if (parts.size() > 2) {
			vector<string> parameters = explode(parts[2], ' ');

			for (unsigned int i = 0; i < parameters.size(); i++) {
				string attr, val;
				extractValue(parameters[i], attr, val);	
				if (attr != "sync_type" && attr != "map_type") {
					attributes[attr] = atoi(val.c_str());
				}
			}	
		}

		ompt_event_s *event = NULL;
	
		try {	
			// get the corresponding enum item (EventType) of the string (easier comparisons)
			EventType event_type = eventtype_map.at(event_name);
			switch (event_type) {
				// TODO: Add optional callback events
				
				// ompt_new_parallel_callback_t
				case ompt_event_parallel_begin:	
					{
						ompt_new_parallel_s *e = new ompt_new_parallel_s;
						e->parent_task_id = attributes.at("parent_task_id");
						e->parent_task_frame = (ompt_frame_t*) attributes.at("parent_task_frame");
						e->parallel_id = attributes.at("parallel_id");
						e->parallel_function = (void*) attributes.at("parallel_function");

						event = e;
					}
					break;	

				// ompt_parallel_callback_t
				case ompt_event_parallel_end:
					{
						ompt_parallel_s *e = new ompt_parallel_s;
						e->parallel_id = attributes.at("parallel_id");
						e->task_id = attributes.at("task_id");
						event = e;
					}
					break;


				// ompt_new_task_callback_t
				case ompt_event_task_begin:
					{
						ompt_new_task_s *e = new ompt_new_task_s;
						e->parent_task_id = attributes.at("parent_task_id");
						e->parent_task_frame = (ompt_frame_t*) attributes.at("parent_task_frame");
						e->new_task_id = attributes.at("new_task_id");
						e->new_task_function = (void*) attributes.at("new_task_function");
						event = e;
					}
					break;

				// ompt_task_callback_t
				case ompt_event_task_end:
					{
						ompt_task_s *e = new ompt_task_s;
						e->task_id = attributes.at("task_id");
						event = e;
					}
					break;

				// ompt_thread_callback_t
				case ompt_event_thread_begin:
				case ompt_event_thread_end:
					{
						ompt_thread_type_s *e = new ompt_thread_type_s;
						e->thread_type = (ompt_thread_type_e) attributes.at("thread_type");
						e->thread_id = attributes.at("thread_id");
						event = e;
					}
					break;

				// OMPT 4.0
				// ompt_new_target_callback_t
				case ompt_event_target_begin:
				case ompt_event_target_update_begin:
				case ompt_event_target_invoke_begin:
					{
						ompt_new_target_s *e = new ompt_new_target_s;
						e->task_id = attributes.at("task_id");
						e->target_id = attributes.at("target_id");
						e->device_id = attributes.at("device_id");	
						event = e;
					}
					break;

				// ompt_target_callback_t
				case ompt_event_target_end:
				case ompt_event_target_update_end:
				case ompt_event_target_invoke_end:
					{
						ompt_target_s *e = new ompt_target_s;
						e->task_id = attributes.at("task_id");
						e->target_id = attributes.at("target_id"); 
						event = e;
					}
					break;

				// ompt_data_map_callback_t
				case ompt_event_data_map_end:
					{
						ompt_data_map_s *e =  new ompt_data_map_s;

						e->task_id = attributes.at("task_id");
						e->target_id =  attributes.at("target_id");
						e->data_map_id = attributes.at("data_map_id");
						event = e;
					}
					break;
				
				// ompt_new_data_map_callback_t
				case ompt_event_data_map_begin:
					{
						ompt_new_data_map_s *e = new ompt_new_data_map_s;
						e->task_id = attributes.at("task_id");
						e->target_id =  attributes.at("target_id");
						e->data_map_id = attributes.at("data_map_id");
						e->device_id =  attributes.at("device_id");
						event = e;
					}
					break;
			}

			if (event != NULL) {
				event->type = event_type;
				event->name = event_name;

				this->eventList.add(event);
			}

		} catch (const out_of_range& oor) {
			// event not defined in event.h
			cerr << "Unknown event (" << event_name << ") or attribute found: " << oor.what() << endl;
		}

		
	}
}


EventList Parser::getEventList() {
	return this->eventList;
}
