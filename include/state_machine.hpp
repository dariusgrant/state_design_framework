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
template <class ObjType, class Input> class State {
  // template <class Input> class State {
protected:
  ObjType *object;

public:
  /*
  Parameters:
    object: The address of the object the State operates on.
  */
  State(ObjType *object) : object(object) {}

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
protected:
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

  /*
  Description:
  The FSM receives an input, performs its exit function, transition to next
  state defined by the transition mapping, and finally enters the state using
  the same input.

  Parameters:
    input: The input that will be passed to the states to process.
  */
  void input(Input input) {
    throw_if_state_null();
    try {
      auto state_transitions = transitions.at(state);
      auto new_state = state_transitions.at(input);
      state->on_exit(input);
      state = new_state;
      state->on_enter(input);
    } catch (std::out_of_range &e) {
      throw e;
    }
  }

protected:
  /*
Description:
The FSM receives an input and enters the state it's currently set to. This can
be used to effectively start a process without needing external forces to
invoke the machine's behaviors.

Parameters:
  input: The input that will be passed to the state to process.
*/
  void start(Input input) {
    throw_if_state_null();
    state->on_enter(input);
  }

  /*
  Description:
  The FSM receives an input and exits the state it's currently set to.
  Afterwards, the state of the FSM is null, effectively communicating that the
  machine has stopped and will need to be reset.

  Parameters:
    input: The input that will be passed to the state to process.
  */
  void stop(Input input) {
    throw_if_state_null();
    state->on_exit(input);
    state = nullptr;
  }

  void reset(BaseStateClass *initial_state,
             std::unordered_map<BaseStateClass *,
                                std::unordered_map<Input, BaseStateClass *>>
                 transitions) {
    state = initial_state;
    this->transitions = transitions;
  }

  void throw_if_state_null() {
    if (!state) {
      throw std::runtime_error("FSM state is null.");
    }
  }
};