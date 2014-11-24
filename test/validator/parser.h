#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <map>
#include <stdexcept>

#include "event.h"
#include "eventlist.h"

#ifndef PARSER_H
#define PARSER_H

using namespace std;

/**
 * Parses input and puts it into a EventList.
 */

class Parser {
	private:
		EventList eventList;

		/**
 		 * splits a string by using the passed delimiter
 		 */
		vector<string> explode(const string &str, char delim);
		
		/**
		 * removes all whitespaces in a string
		 */
		string removeSpaces(string str);

		/**
		 * extracts attribute and value, e.g. task_id=1 -> task_id, 1
 		 */
		void extractValue(const string &str, string &attr, string &val);
	public:
		/**
 		 * parse line to extract event (if found) out of it
		 */
		void parseLine(const string &str);


		/**
		 * returns list of the parsed events
 		 */
		EventList getEventList();
};

#endif
