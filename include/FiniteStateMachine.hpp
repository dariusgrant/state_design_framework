#pragma once

#include "State.hpp"
#include "utility/Environment.hpp"
#include "utility/StateType.hpp"
#include <cstddef>
#include <fmtmsg.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <variant>

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
  static_assert((std::is_base_of_v<state_t<_Obj>, _S0> ||
                 std::is_base_of_v<final_state_t<_Obj>, _S0>) ||
                    ((std::is_base_of_v<state_t<_Obj>, _Sn> ||
                      std::is_base_of_v<final_state_t<_Obj>, _Sn>) ||
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
  static constexpr bool has_final_state =
      (std::is_base_of_v<fsm::final_state_t<_Obj>, _S0> ||
       (std::is_base_of_v<fsm::final_state_t<_Obj>, _Sn> || ...));

protected:
  shared_ptr_t _object;  // The object being managed by the FSM
  state_tuple_t _states; // The states of the objected
  state_address_hash_map_t
      _state_address_hash_map; // State type hash to state address
  typename state_address_hash_map_t::iterator
      _current_state_address_iterator; // The current state's iterator
  size_t _current_state_hash;

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
            _state_address_hash_map.find(state_type_hash_v<_S0>)),
        _current_state_hash(state_type_hash_v<_S0>) {
    if (_current_state_address_iterator == _state_address_hash_map.end()) {
      abort();
    }
    auto initial_state = std::get<0>(_states);
    std::ostringstream os;
    os << "initial state: " << typeid(initial_state).name() << " - "
       << initial_state.get_hash();
    fmtmsg(MM_SOFT | MM_UTIL | MM_PRINT | MM_RECOVER, "FSM:process", MM_INFO,
           os.str().data(), nullptr, nullptr);
  }

  operator _Obj &() { return *_object; }
  _Obj &operator*() const { return *_object; }
  _Obj *operator->() const { return _object.get(); }

  template <typename... _Args> FiniteStateMachine &process(_Args... args) {
    if (_terminated) {
      if constexpr (fsm::environ::THROW_ON_PROCESS_AFTER_TERMINATION) {
        throw std::runtime_error("FSM already terminated.");
      }
      return *this;
    }

    if (_started) {
      _process_state(args...);
    } else {
      start();
      _process_state(args...);
    }
    return *this;
  }

  template <typename... _ObjArgs> FiniteStateMachine &reset(_ObjArgs... args) {
    *this = FiniteStateMachine(args...);
    return *this;
  }

  bool in_final_state() const {
    return std::visit([&](auto &s) { return s->is_final; },
                      _current_state_address_iterator->second);
  }

  std::size_t get_current_state_hash() const { return _current_state_hash; }

private:
  template <typename... _Args> void _process_state(_Args... args) {
    std::visit(
        [&](auto &s) {
          // Process current state and get the next state's hash
          auto next_state_hash = s->process(args...);

          // Set current state to the next state
          _current_state_address_iterator =
              _state_address_hash_map.find(next_state_hash);

          _current_state_hash = next_state_hash;

          if constexpr (fsm::environ::debug) {
            _debug_print_transition(state_type_hash_v<decltype(s)>);
          }
        },
        _current_state_address_iterator->second);
  }

  void _debug_print_transition(size_t previous_state_hash) {
    std::ostringstream os;
    os << "previous_state: " << _debug_state_name(previous_state_hash)
       << ", current_state: " << _debug_current_state_name() << "\n";
    fmtmsg(MM_SOFT | MM_UTIL | MM_PRINT | MM_RECOVER, "FSM:process", MM_INFO,
           os.str().data(), nullptr, nullptr);
  }

  const char *_debug_state_name(const size_t &state_hash) {
    return typeid(_state_address_hash_map[state_hash]).name();
  }

  const char *_debug_current_state_name() {
    return std::visit([&](auto &s) { return typeid(s).name(); },
                      _current_state_address_iterator->second);
  }
};

template <class _S0, class... _Sn>
class Acceptor : protected FiniteStateMachine<std::nullptr_t, _S0, _Sn...> {
public:
  using FiniteStateMachine<std::nullptr_t, _S0, _Sn...>::reset;

public:
  Acceptor() : FiniteStateMachine<std::nullptr_t, _S0, _Sn...>() {}

  /*
    Check if a given sequence of input is accepted.
  */
  template <typename _Arg, typename... _Args>
  bool is_sequence_accepted(_Arg arg, _Args... args) {
    this->process(arg);
    if (sizeof...(args) > 0) {
      return try_accept(args...);
    } else {
      return this->in_final_state();
    }
  }

  bool is_accepted() { return this->in_final_state(); }

  /*
  Check if an input is accepted.
  */
  template <typename _Arg, typename... _Args>
  bool is_accepted(_Arg arg, _Args... args) {
    this->process(arg, args...);
    return this->in_final_state();
  }
};
} // namespace fsm
