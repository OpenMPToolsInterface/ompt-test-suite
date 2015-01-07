#ifndef states_h
#define	states_h

#include <vector>
#include <string>

#include <omp.h>
#include <ompt.h>

using namespace std;

std::string state_string(vector<ompt_state_t>& observed_states);
void print_current_states(vector<ompt_state_t>& observed_states);
bool check_states(vector<ompt_state_t>& observed_states, string regex_pattern);
void check_current_state(ompt_state_t expected_state);

#endif /* states_h */
