#include <stdexcept>
#include <unordered_map>

/*
Description:
State
*/
template <class Input> class State {
public:
  virtual void on_enter(Input input) {};
  virtual void on_exit(Input input) {};
};

/*
Description:
FSM (Finite State Machine) contains a set of states that are able to process the
same set of inputs that are used to transition from state to state and perform
actions dependent on the state.

Parameters:
  BaseStateClass: The base class of the set of states
  the FSM will use for transitions.

  Input: The input "class" - the type that's used for input transitions where
  its value is the key within the transition mapping.
*/
template <class BaseStateClass, class Input> class FSM {
  BaseStateClass *state;
  std::unordered_map<BaseStateClass *,
                     std::unordered_map<Input, BaseStateClass *>>
      transitions;

public:
  FSM(BaseStateClass *initial_state,
      std::unordered_map<BaseStateClass *,
                         std::unordered_map<Input, BaseStateClass *>>
          transitions)
      : state(initial_state), transitions(transitions) {}

  void input(Input input) {
    try {
      auto state_transitions = transitions.at(state);
      auto new_state = state_transitions.at(input);
      state->on_exit(input);
      state = new_state;
      state->on_enter(input);
    } catch (std::out_of_range &e) {
      throw e;
      // throw std::out_of_range("Failed to find transition for " + state + " X
      // " + input + ": " + e.what());
    }
  }
};