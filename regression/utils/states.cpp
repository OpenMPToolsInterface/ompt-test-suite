#include <map>
#include <vector>
#include <iostream>
#include <string>
#include <iostream>

#include "states.h"
#include "regex-match.h"

/*
 * check whether the state sequence in vector observed_states match a regular 
 * expression specified in regex_pattern
 */ 
bool
check_states(vector<ompt_state_t>& observed_states, string regex_pattern)
{
    map<ompt_state_t, string> map; 
    #define macro( state, id) map[state] = #state;
    FOREACH_OMPT_STATE( macro )
    #undef macro
    std::string observed_states_str;
    for (int i = 0; i < observed_states.size(); i++) {
       if (map[observed_states[i]].compare("ompt_state_overhead") != 0 && 
            (i == 0 || observed_states[i] != observed_states[i-1] )) {
            observed_states_str.append(map[observed_states[i]]); 
       }
    }
    //cout << "observed_states" << endl;
    //cout << observed_states_str << endl;
    //cout << "regex_pattern" << endl;
    //cout << regex_pattern << endl;
    //cout << "regex_match_result" << endl;
    //cout << regex_match(observed_states_str, regex_pattern) << "\n" << endl;
    if (regex_match(observed_states_str, regex_pattern) == 0) {
        return true;
    }
    return false;
}

void 
print_current_states(vector<ompt_state_t>& observed_states)
{
    printf("observed_states size: %d \n", observed_states.size());
    map<ompt_state_t, string> map; 
    #define macro( state, id) map[state] = #state;
    FOREACH_OMPT_STATE( macro )
    #undef macro

    std::string observed_states_str;
    for (int i = 0; i < observed_states.size(); i++) {
        cout << map[observed_states[i]] << " \n"; 
    }
}
