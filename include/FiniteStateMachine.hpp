#pragma once

#include "AbstractState.hpp"
#include "ProcessStrategy.hpp"
#include "SharedOutput.hpp"
#include "utility/StateType.hpp"
#include <algorithm>
#include <execution>
#include <functional>
#include <future>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <variant>

namespace fsm {
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
      _current_state; // Variants of state pointers
  FutureSubscription<_Obj> _shared_output;
  bool _started;    // Has the FSM started?
  bool _terminated; // Has the FSM been terminated?

public:
  template <typename... _ObjArgs>
  FiniteStateMachine(_ObjArgs... args)
      : _object(std::make_shared<_Obj>(args...)),
        _states(std::make_tuple(_S0(_object), _Sn(_object)...)),
        _current_state(&std::get<0>(_states)), _started(false),
        _terminated(false) {}

  // Accessors
  const _Obj &get() const { return *_object; }

  // Queries
  constexpr bool is_terminable() const { return has_terminal_state; }
  bool is_terminated() const { return _terminated; }

  template <class _Fsm> FiniteStateMachine &hook(std::string name, _Fsm &fsm) {
    _shared_output.add(name, fsm);
    return *this;
  }

  template <typename _Strat = DefaultProcessStrategy, typename... _Args>
  FiniteStateMachine &process(_Args... args) {
    static_assert(std::is_base_of_v<AbstractProcessStrategy, _Strat>,
                  "`_Strat` must derive from `AbstractProcessStrategy`");
    if (_terminated) {
      throw std::runtime_error("FSM already terminated.");
    }

    if (_started) {
      _Strat::process(std::function<void(_Args...)>(
                          [&](_Args... args) { this->_enter_state(args...); }),
                      std::function<void(_Args...)>(
                          [&](_Args... args) { this->_exit_state(args...); }),
                      std::function<void(_Args...)>([&](_Args... args) {
                        this->_transition_state(args...);
                      }),
                      args...);
    } else {
      _started = true;
      _enter_state(args...);
    }
    return *this;
  }

private:
  template <typename... _Args> void _enter_state(_Args... args) {
    std::shared_future<_Obj> future_obj;
    std::visit(
        [&](auto &s) {
          future_obj = std::async(std::launch::deferred, [&]() -> _Obj {
            s->enter(args...);
            if (s->is_terminal) {
              _terminated = true;
            }
            return *_object;
          });
        },
        _current_state);

    _notify_subscribers(future_obj);
    *_object = future_obj.get();
  }

  template <typename... _Args> void _exit_state(_Args... args) {
    std::visit([&](auto &s) { s->exit(args...); }, _current_state);
  }

  template <typename... _Args> void _transition_state(_Args... args) {
    _current_state = std::visit(
        [&](auto &s) {
          auto next_state_identity = s->transition(args...);
          return &std::get<typename decltype(next_state_identity)::type>(
              _states);
        },
        _current_state);
  }

  void _notify_subscribers(std::shared_future<_Obj> future_obj) {
    std::for_each(std::execution::par_unseq, _shared_output.begin(),
                  _shared_output.end(),
                  [&](auto &sub_cb) { sub_cb.second(future_obj); });
  }
};
} // namespace fsm
