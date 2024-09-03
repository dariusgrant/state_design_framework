#include <stdexcept>
#include <unordered_map>

/*
Description:
State is a template of instances representing a state of a machine. An example
would be the state of a turnstile (https://en.wikipedia.org/wiki/Turnstile):
Locked or Unlocked.

Parameters:
  ObjType: The type of object the State operates on.
  Input: The type of paramter the State is constrained to.
*/
// template <class ObjType, class Input> class State {
template <class Input> class State {
private:
  // ObjType *object;

public:
  /*
  Parameters:
    object: The address of the object the State operates on.
  */
  // State(ObjType *object) : object(object) {}

  /*
  Description:
  What should happened when the State is entered/started.
    input: The input used to enter/start the State.
  */
  virtual void on_enter(Input input) = 0;

  /*
  Description:
  What should happened when the State is exited/finished.

  Parameters:
    input: The input used to exit/finish the State.
  */
  virtual void on_exit(Input input) = 0;
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
  BaseStateClass *state; // The currenet state of the FSM.
  std::unordered_map<BaseStateClass *,
                     std::unordered_map<Input, BaseStateClass *>>
      transitions; // The transitional mappings from state to state.

public:
  FSM() : state(nullptr), transitions({}) {}
  /*
  Parameters:
    intitial_state: Address of the initial state the FSM will start in.
    transitions: The transitional mapping of states.
  */
  FSM(BaseStateClass *initial_state,
      std::unordered_map<BaseStateClass *,
                         std::unordered_map<Input, BaseStateClass *>>
          transitions)
      : state(initial_state), transitions(transitions) {}

  void reset(BaseStateClass *initial_state,
             std::unordered_map<BaseStateClass *,
                                std::unordered_map<Input, BaseStateClass *>>
                 transitions) {
    state = initial_state;
    this->transitions = transitions;
  }
  /*
  Description:
  The FSM receives an input, performs its exit function, transitions

  Parameters:
    BaseStateClass: The base class of the set of states
    the FSM will use for transitions.

    Input: The input "class" - the type that's used for input transitions
  where its value is the key within the transition mapping.
  */
  void input(Input input) {
    try {
      if (!state) {
        throw std::runtime_error("FSM state is null.");
      }
      auto state_transitions = transitions.at(state);
      auto new_state = state_transitions.at(input);
      state->on_exit(input);
      state = new_state;
      state->on_enter(input);
    } catch (std::out_of_range &e) {
      throw e;
    }
  }
};