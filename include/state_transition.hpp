#include <string>

/*
A "description" of a state transition occurrence.
*/
template <class obj_t> struct state_transition {
  std::string from_state;
  std::string to_state;
  std::string reason;

  state_transition(std::string from_state, std::string to_state,
                   std::string reason = "")
      : from_state(from_state), to_state(to_state), reason(reason) {}
};