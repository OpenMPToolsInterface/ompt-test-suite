#include "main.h"

using namespace std;

int main(int argc, char* argv[]) {

	string line;
	Parser p;

	bool silent = false;
	for (int i = 1; i < argc; i++) {
		if (string(argv[i]) == "--silent") {
			silent = true;
		}
	}

	cout << "\nValidator running...\n\n";

	// parse input
	while (getline(cin, line)) {
		if (!silent) { 	
			cout << line << endl;
		}

		p.parseLine(line);
	}

	cout << "\nTesting...\n\n";

	// run all tests
	if (Tests::check_parallel_begin_end(p.getEventList())) {
		cout << "Test 'parallel begin / end' passed" << endl;
	} else {
		cout << "Test 'parallel begin / end' failed" << endl;
	}

	if (Tests::check_target_begin_end(p.getEventList())) {
		cout << "Test 'target begin / end' passed" << endl;
	} else {
		cout << "Test 'target begin / end' failed" << endl;
	}	

	if (Tests::check_data_map_begin_end(p.getEventList())) {
		cout << "Test 'data map begin / end' passed" << endl;
	} else {
		cout << "Test 'data map begin / end' failed" << endl;
	}
	
	if (Tests::check_task_ids(p.getEventList())) {
		cout << "Test 'check task ids' passed" << endl;
	} else {
		cout << "Test 'check task ids' failed" << endl;
	}
}
