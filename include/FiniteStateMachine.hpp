#include "AbstractState.hpp"
#include <type_traits>
#include <utility>
#include <variant>
/*
PROTOTYPE 2

Limitations:
    - States must be default-constructible
        - No easy way to pass arguments in constructor
        - Alternative was have separate method for adding states with variadic
parameters
            - Constructing within the method was not possible for the initial
state
                - `std::variant` requires the first template parameter to be
default constructible
        - Decided to add a `configure` method for states to achieve this after
*/

/*
Class `AbstractState` provides an interface for state implementation.
- `enter()` allows the state to modify the object's state. It is intentionally
  the only function that allows modification on the passed object.
- `exit()` purpose exists primarily for state cleanup but isn't bounded.
- `transition()` implements the theoretical transition function.
- `configure()` enables the ability to configure the state. This overcomes the
  limitation of the class `FiniteStateMachine` (FSM) requiring states to be
  default constructible.

Below is the function call order on a state when operating under a FSM:
    - Initial State: `enter()` -> `exit()` -> `transition()`
    - Non-initial State: `exit()` -> `transition()` -> `enter()`

With the exception of `configure()`, an object and input is always passed
through each function. However, `enter()` is the only function that can
manipulate the state of the passed object. This is to simplify modification on
the object and limit the scope of what the state should do compared to the next
state after transition.

`exit()` occurs right before a transition occurs. The original intent behind its
addition was to have the state cleanup itself before transitioning out from it.
However, the limitation is only philosophical and cannot be enforced. There are
also valid use-cases where state may want to be preserved through, such as
simply keeping a counter.

`transition()` references a mapping of states as a parameter. The mapping SHOULD
be the only reference to other states

Make StopLight::public fsm<LightState>
    - What makes a light state a light state
        - Can ask what the state's color
*/

namespace fsm {
template <class ObjType, class... StateTypes> class FiniteStateMachine {
  static_assert((std::is_base_of_v<AbstractState, StateTypes> && ...),
                "The state does not derived from class `AbstractState`");

private:
  ObjType object;
  StateMap<StateTypes...> _states;
  std::variant<StateTypes...> _curr_state;
  bool _started;

public:
  template <typename... ObjArgTypes>
  FiniteStateMachine(ObjArgTypes... obj_args)
      : object(obj_args...),
        _states({{GetStateTypeIndex<StateTypes>(),
                  std::variant<StateTypes...>(StateTypes())}...}),
        _started(false) {}

  template <typename InputType> FiniteStateMachine &start(InputType input) {
    if (_started) {
      throw StartedFSM();
    }
    std::visit([this, &input](auto &s) { return s.enter(object, input); },
               _curr_state);
    _started = true;
    return *this;
  }

  template <class State, typename T, typename... Ts>
  FiniteStateMachine &configure(T arg, Ts... args) {
    std::visit([this, &arg, &args...](auto &s) { s.configure(arg, args...); },
               _states[GetStateTypeIndex<State>()]);
    return *this;
  }

  template <typename T> FiniteStateMachine &process(T input) {
    if (!_started) {
      throw UnstartedFSM();
    }
    _curr_state = std::visit(
        [this, &input](auto &s) {
          auto &obj = std::as_const(object);
          s.exit(obj, input);
          return s.transition(std::as_const(object), input, _states);
        },
        _curr_state);
    std::visit([this, &input](auto &s) { s.enter(object, input); },
               _curr_state);
    return *this;
  }

  template <typename T, typename... Ts>
  FiniteStateMachine &process(T input, Ts... inputs) {
    process(input).process(inputs...);
    return *this;
  }

  const ObjType &get_object() { return object; }
};
}; // namespace fsm
