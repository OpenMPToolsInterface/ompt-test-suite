#include <map>
#include <vector>
#include <iostream>
#include <string>
#include <iostream>

#include "states.h"
#include "regex-match.h"


std::string 
state_string(vector<ompt_state_t>& observed_states)
{
  static map<ompt_state_t, string> map; 
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
  return observed_states_str;
}

/*
 * check whether the state sequence in vector observed_states match a regular 
 * expression specified in regex_pattern
 */ 
bool
check_states(vector<ompt_state_t>& observed_states, string regex_pattern)
{
  return (regex_match(state_string(observed_states), regex_pattern) == 0);
}

void 
print_current_states(vector<ompt_state_t>& observed_states)
{
  cout << "observed_states size: " << observed_states.size() << endl;
  map<ompt_state_t, string> map; 
  #define macro( state, id) map[state] = #state;
  FOREACH_OMPT_STATE( macro )
  #undef macro

  std::string observed_states_str;
  for (int i = 0; i < observed_states.size(); i++) {
      cout << map[observed_states[i]] << " \n"; 
  }
}
