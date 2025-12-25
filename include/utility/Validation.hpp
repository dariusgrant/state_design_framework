#pragma once

#include "../FiniteStateMachine.hpp"
#include "StateType.hpp"
#include <cstddef>
#include <functional>
#include <tuple>
#include <type_traits>

namespace fsm::validation {
template <typename... Inputs> struct Expected {
  size_t initial_state;
  size_t next_state;
  std::tuple<Inputs...> inputs;

  Expected(size_t initial_state, size_t next_state, Inputs... inputs)
      : initial_state(initial_state), next_state(next_state),
        inputs(inputs...) {}
};

/*
`Validator`: Validates a FSM state after taking input and managed object.
*/
template <class _Fsm> class Validator {
  static_assert(std::is_base_of_v<fsm::BaseFiniteStateMachine, _Fsm>);
  using check_func_t = std::function<bool(const typename _Fsm::obj_t &)>;

  _Fsm &_fsm;

public:
  Validator(_Fsm &fsm) : _fsm(fsm) {}

  // Check the current state of the FSM.
  bool check(size_t current_state) {
    return current_state == _fsm.get_current_state_hash();
  }

  // Check the current state of the FSM.
  template <class _ExpectedState> bool check() {
    return check(state_type_hash_v<_ExpectedState>);
  }

  // Check the managed object of the FSM.
  bool check(check_func_t check_func) { return check_func(_fsm); }

  // Check the current state and the managed object of the FSM.
  template <class _ExpectedState> bool check(check_func_t check_func) {
    return check(state_type_hash_v<_ExpectedState>) && check(check_func);
  }

  // Test the FSM's expected current state and the expected state after
  // processing input.
  template <typename... _Inputs>
  bool test(size_t current_state, size_t next_state, _Inputs... inputs) {
    if (!check(current_state)) {
      return false;
    }
    _fsm.process(inputs...);
    return check(next_state);
  }

  // Test the FSM's expected current state and the expected state after
  // processing input.
  template <class _InitialState, class _ExpectedState, typename... _Inputs>
  bool test(_Inputs... inputs) {
    return test(fsm::state_type_hash_v<_InitialState>,
                fsm::state_type_hash_v<_ExpectedState>, inputs...);
  }

  // Test the FSM's expected current state and the expected state after
  // processing input. Additionally, check the expected managed object of the
  // FSM.
  template <class _InitialState, class _ExpectedState, typename... _Inputs>
  bool test(check_func_t check_func, _Inputs... inputs) {
    return test<_InitialState, _ExpectedState>(inputs...) && check(check_func);
  }
};
} // namespace fsm::validation