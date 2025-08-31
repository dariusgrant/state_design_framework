#include "AbstractState.hpp"
#include "utility/Log.hpp"
#include "utility/StateType.hpp"
#include "utility/String.hpp"
#include <future>
#include <stdexcept>
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

`transition()` must return the type index of a state that's apart of the FSM.

Make StopLight::public fsm<LightState>
    - What makes a light state a light state
        - Can ask what the state's color
*/

namespace fsm {
template <class ObjType, class... StateTypes> class FiniteStateMachine {
  // Reject parameter type that isn't a state.
  static_assert(((std::is_base_of_v<NonTerminalState, StateTypes> ||
                  std::is_base_of_v<TerminalState, StateTypes>) &&
                 ...),
                "The state does not derived from class `AbstractState`");

public:
  static constexpr bool has_terminal_state =
      (std::is_base_of_v<fsm::TerminalState, StateTypes> || ...);

private:
  ObjType _object;                 // The object that's managed by the FSM
  StateMap<StateTypes...> _states; // The set of states the object can be in
  std::variant<StateTypes...>
      _current_state; // The current state the object is in
  bool _started;      // Has the FSM started?
  bool _terminated;   // Has the FSM been terminated?
  std::shared_future<ObjType> _after_enter; // A future copy of the object after
                                            // a state completion event
  std::function<void(std::shared_future<ObjType>)>
      _future_object_callback; // The callback for handling the future copies of
                               // the object upon state completion

  // TODO move to a stats struct
  uint64_t _invalid_shared_state_count;

public:
  template <typename... ObjArgTypes>
  FiniteStateMachine(ObjArgTypes... obj_args)
      : _object(obj_args...),
        _states({{GetStateTypeIndex<StateTypes>(),
                  std::variant<StateTypes...>(StateTypes())}...}),
        _started(false), _terminated(false), _future_object_callback(),
        _invalid_shared_state_count(0) {}

  // Accessors
  const ObjType &get() const { return _object; }

  // Queries
  constexpr bool is_terminable() const { return has_terminal_state; }
  const bool is_terminated() const { return _terminated; }

  // Modifiers
  FiniteStateMachine &
  set_future_object_cb(std::function<void(std::shared_future<ObjType>)> cb) {
    // debug_log(*this, __func__,
    //           key_value_string("_future_object_callback",
    //           typeid(cb).name()));
    _future_object_callback = cb;
    return *this;
  }

  template <typename InputType> FiniteStateMachine &start(InputType input) {
    // debug_log(*this, __func__, key_value_string("input", input));
    if (_started) {
      throw std::logic_error("FSM has already been started");
    }

    _enter_state(input);
    return *this;
  }

  template <class State, typename T, typename... Ts>
  FiniteStateMachine &configure(T arg, Ts... args) {
    std::visit([this, &arg, &args...](auto &s) { s.configure(arg, args...); },
               _states[GetStateTypeIndex<State>()]);
    return *this;
  }

  template <typename T> FiniteStateMachine &process(T input) {
    // debug_log(*this, __func__, key_value_string("input", input));
    if (!_started) {
      throw std::logic_error("FSM hasn't been started");
    }

    if (_terminated) {
      throw std::runtime_error("FSM already terminated.");
    }
    _exit_state(input);
    _enter_state(input);
    return *this;
  }

  template <typename T, typename... Ts>
  FiniteStateMachine &process(T input, Ts... inputs) {
    process(input).process(inputs...);
    return *this;
  }

private:
  template <typename T> void _enter_state(T &input) {
    // debug_log(*this, __func__, key_value_string("input", input));
    std::shared_future<ObjType> sf;
    std::visit(
        [this, &input, &sf](auto &s) {
          sf = std::async(std::launch::deferred, [&]() -> ObjType {
            s.enter(_object, input);
            if (s.is_terminal) {
              _terminated = true;
            }
            return _object;
          });

          // If there is a callback defined,
          // then pass the shared future object
          // to it. This is to let the user
          // decide how to handle the distribution
          // to other objects, potentially in other
          // threads.
          if (_future_object_callback) {
            _future_object_callback(sf);
          }
        },
        _current_state);

    if (!_started) [[unlikely]] {
      _started = true;
    }

    if (!sf.valid()) [[unlikely]] {
      _invalid_shared_state_count += 1;
    } else {
      _object = sf.get();
    }
  }

  template <typename T> void _exit_state(T &input) {
    // debug_log(*this, __func__, key_value_string("input", input));
    _current_state = std::visit(
        [this, &input](auto &s) {
          auto &obj = std::as_const(_object);
          s.exit(obj, input);
          auto next_state_type = s.transition(std::as_const(_object), input);
          return _states[next_state_type];
        },
        _current_state);
  }
};
}; // namespace fsm
