#pragma once

#include "AbstractState.hpp"
#include "utility/Log.hpp"
#include "utility/StateType.hpp"
#include "utility/String.hpp"
#include <functional>
#include <future>
#include <memory>
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
struct AbstractProcessStrategy {
  template <typename... _Args> using callback_t = std::function<void(_Args...)>;

  template <typename... _Args>
  constexpr static void
  process(callback_t<_Args...> enter, callback_t<_Args...> exit,
          callback_t<_Args...> transition, _Args... args) {
    static_assert(
        std::false_type::value,
        "`AbstractProcessStrategy` cannot be used as a process strategy.");
  };
};

struct DefaultProcessStrategy : public AbstractProcessStrategy {
  template <typename... _Args>
  constexpr static void
  process(callback_t<_Args...> enter, callback_t<_Args...> exit,
          callback_t<_Args...> transition, _Args... args) {
    exit(args...);
    transition(args...);
    enter(args...);
  };
};

template <class _Obj, class _S0, class... _Sn> class FiniteStateMachine {
  static_assert(std::is_constructible_v<_S0, _S0> &&
                (std::is_constructible_v<_Sn, _Sn> && ...));

  static_assert(std::is_assignable_v<_S0 &, _S0> &&
                (std::is_assignable_v<_Sn &, _Sn> && ...));

  // Reject parameter type that isn't a state.
  static_assert(((std::is_base_of_v<NonTerminalState<_Obj>, _Sn> ||
                  std::is_base_of_v<TerminalState<_Obj>, _Sn>) &&
                 ...),
                "The state does not derived from class `AbstractState`");

public:
  static constexpr bool has_terminal_state =
      (std::is_base_of_v<fsm::TerminalState<_Obj>, _S0> ||
       (std::is_base_of_v<fsm::TerminalState<_Obj>, _Sn> || ...));

private:
  std::shared_ptr<_Obj> _object; // The object that's managed by the FSM
  // StateMap<_S0, _Sn...>
  StateTuple<_S0, _Sn...> _states; // The set of states the object can be in
  StateVariantType<_S0 *, _Sn *...>
      _current_state;                    // Variants of state pointers
  bool _started;                         // Has the FSM started?
  bool _terminated;                      // Has the FSM been terminated?
  std::shared_future<_Obj> _after_enter; // A future copy of the object after
                                         // a state completion event
  std::function<void(std::shared_future<_Obj>)>
      _future_object_callback; // The callback for handling the future copies of
                               // the object upon state completion

public:
  template <typename... ObjArgTypes>
  FiniteStateMachine(ObjArgTypes... obj_args)
      : _object(std::make_shared<_Obj>(obj_args...)),
        _states(std::make_tuple(_S0(_object), _Sn(_object)...)),
        _current_state(&std::get<0>(_states)), _started(false),
        _terminated(false), _future_object_callback() {
    // std::visit([](auto &s) { std::cout << typeid(s).name() << "\n"; },
    //            _current_state);
  }

  // Accessors
  const _Obj &get() const { return *_object; }

  // Queries
  constexpr bool is_terminable() const { return has_terminal_state; }
  const bool is_terminated() const { return _terminated; }

  // Modifiers
  FiniteStateMachine &
  set_future_object_cb(std::function<void(std::shared_future<_Obj>)> cb) {
    // debug_log(*this, __func__,
    //           key_value_string("_future_object_callback",
    //           typeid(cb).name()));
    _future_object_callback = cb;
    return *this;
  }

  template <typename... T> FiniteStateMachine &start(T... inputs) {
    // debug_log(*this, __func__, key_value_string("input", input));
    if (_started) {
      throw std::logic_error("FSM has already been started");
    }

    _started = true;
    _enter_state(inputs...);
    return *this;
  }

  template <class State, typename T, typename... Ts>
  FiniteStateMachine &configure(T arg, Ts... args) {
    std::visit([this, &arg, &args...](auto &s) { s.configure(arg, args...); },
               _states[GetStateTypeIndex<State>()]);
    return *this;
  }

  template <typename _Strat = DefaultProcessStrategy, typename... T>
  FiniteStateMachine &process(T... inputs) {
    // debug_log(*this, __func__, key_value_string("input", input));
    static_assert(std::is_base_of_v<AbstractProcessStrategy, _Strat>,
                  "`_Strat` must derive from `AbstractProcessStrategy`");
    if (!_started) {
      throw std::logic_error("FSM hasn't been started");
    }

    if (_terminated) {
      throw std::runtime_error("FSM already terminated.");
    }

    _Strat::process(std::function<void(T...)>(
                        [&](T... args) { this->_enter_state(args...); }),
                    std::function<void(T...)>(
                        [&](T... args) { this->_exit_state(args...); }),
                    std::function<void(T...)>(
                        [&](T... args) { this->_transition_state(args...); }),
                    inputs...);
    return *this;
  }

private:
  template <typename... T> void _enter_state(T... inputs) {
    std::cout << "Printing enter state\n";
    // debug_log(*this, __func__, key_value_string("input", inputs)...);
    std::shared_future<_Obj> sf;
    std::visit(
        [this, &inputs..., &sf](auto &s) {
          sf = std::async(std::launch::deferred, [&]() -> _Obj {
            s->enter(inputs...);
            if (s->is_terminal) {
              _terminated = true;
            }
            return *_object;
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

    *_object = sf.get();
  }

  template <typename... T> void _exit_state(T... inputs) {
    // debug_log(*this, __func__, key_value_string("input", input));
    std::visit([this, &inputs...](auto &s) { s->exit(inputs...); },
               _current_state);
  }

  template <typename... T> void _transition_state(T... inputs) {
    // debug_log(*this, __func__, key_value_string("input", input));
    _current_state = std::visit(
        [this, &inputs...](auto &s) {
          auto next_state_identity = s->transition(inputs...);
          return &std::get<typename decltype(next_state_identity)::type>(
              _states);
        },
        _current_state);
  }
};
}; // namespace fsm
