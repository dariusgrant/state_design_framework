#pragma once

#include "ProcessStrategy.hpp"
#include "State.hpp"
#include "utility/Environment.hpp"
#include "utility/StateType.hpp"

#include <cstddef>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>

namespace fsm {
class BaseFiniteStateMachine {
protected:
  bool _started;
  bool _terminated;

public:
  BaseFiniteStateMachine() : _started(false), _terminated(false) {}

  bool has_started() const { return _started; }
  bool is_terminated() const { return _terminated; }
  void start() {
    if (!_started) {
      _started = true;
    }
  }
  void terminate() {
    if (_started && !_terminated) {
      _terminated = true;
    }
  }
};

template <class _Obj, class _S0, class... _Sn>
class FiniteStateMachine : public BaseFiniteStateMachine {
  // Reject template type that isn't a state.
  static_assert((std::is_base_of_v<non_terminal_state_t<_Obj>, _S0> ||
                 std::is_base_of_v<terminal_state_t<_Obj>, _S0>) ||
                    ((std::is_base_of_v<non_terminal_state_t<_Obj>, _Sn> ||
                      std::is_base_of_v<terminal_state_t<_Obj>, _Sn>) ||
                     ...),
                "The state does not derived from class `State`");

public:
  using obj_t = _Obj;
  using shared_ptr_t = std::shared_ptr<_Obj>;
  using state_tuple_t = std::tuple<_S0, _Sn...>;
  using state_address_variant_t = std::variant<_S0 *, _Sn *...>;
  using state_address_hash_map_t =
      std::unordered_map<size_t, state_address_variant_t>;

  // Does this FSM contain a terminal state?
  static constexpr bool has_terminal_state =
      (std::is_base_of_v<fsm::terminal_state_t<_Obj>, _S0> ||
       (std::is_base_of_v<fsm::terminal_state_t<_Obj>, _Sn> || ...));

protected:
  shared_ptr_t _object;  // The object being managed by the FSM
  state_tuple_t _states; // The states of the objected
  state_address_hash_map_t
      _state_address_hash_map; // State type hash to state address
  state_address_hash_map_t::iterator
      _current_state_address_iterator; // The current state's iterator

public:
  template <typename... _ObjArgs>
  FiniteStateMachine(_ObjArgs... args)
      : BaseFiniteStateMachine(), _object(std::make_shared<_Obj>(args...)),
        _states(std::make_tuple(_S0(_object), _Sn(_object)...)),
        _state_address_hash_map(
            {{state_type_hash_v<_S0>,
              state_address_variant_t(&std::get<_S0>(_states))},
             {state_type_hash_v<_Sn>,
              state_address_variant_t(&std::get<_Sn>(_states))}...}),
        _current_state_address_iterator(
            _state_address_hash_map.find(state_type_hash_v<_S0>)) {}

  operator _Obj &() { return *_object; }
  _Obj &operator*() const { return *_object; }
  _Obj *operator->() const { return _object.get(); }

  template <typename _Strat = DefaultProcessStrategy, typename... _Args>
  FiniteStateMachine &process(_Args... args) {
    static_assert(std::is_base_of_v<AbstractProcessStrategy, _Strat>,
                  "`_Strat` must derive from `AbstractProcessStrategy`");
    if (_terminated) {
      if constexpr (THROW_ON_PROCESS_AFTER_TERMINATION) {
        throw std::runtime_error("FSM already terminated.");
      }
      return *this;
    }

    if (_started) {
      _Strat::process(std::function<void(_Args...)>(
                          [&](_Args... args) { _enter_state(args...); }),
                      std::function<void(_Args...)>(
                          [&](_Args... args) { _exit_state(args...); }),
                      std::function<void(_Args...)>(
                          [&](_Args... args) { _transition_state(args...); }),
                      args...);
    } else {
      start();
      _enter_state(args...);
    }
    return *this;
  }

  template <typename... _ObjArgs> FiniteStateMachine &reset(_ObjArgs... args) {
    *this = FiniteStateMachine(args...);
    return *this;
  }

private:
  template <typename... _Args> void _enter_state(_Args... args) {
    std::visit(
        [&](auto &s) {
          s->enter(args...);
          if (s->is_terminal) {
            _terminated = true;
          }
        },
        _current_state_address_iterator->second);
  }

  template <typename... _Args> void _exit_state(_Args... args) {
    std::visit([&](auto &s) { s->exit(args...); },
               _current_state_address_iterator->second);
  }

  template <typename... _Args> void _transition_state(_Args... args) {
    std::visit(
        [&](auto &s) {
          // Get the next state's hash
          auto next_state_hash = s->transition(args...);
          _current_state_address_iterator =
              _state_address_hash_map.find(next_state_hash);
        },
        _current_state_address_iterator->second);
  }
};
} // namespace fsm
