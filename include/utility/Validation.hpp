#pragma once

#include "../FiniteStateMachine.hpp"
#include "StateType.hpp"
#include <cstddef>
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

template <class _Fsm> class Validator {
  static_assert(std::is_base_of_v<fsm::BaseFiniteStateMachine, _Fsm>);

  _Fsm &_fsm;

public:
  Validator(_Fsm &fsm) : _fsm(fsm) {}
  template <typename... _Inputs>
  bool test(size_t initial_state, size_t next_state, _Inputs... inputs) {
    auto s = _fsm.get_current_state_hash();
    if (s != initial_state) {
      return false;
    }

    _fsm.process(inputs...);

    if (_fsm.get_current_state_hash() != next_state) {
      return false;
    }

    return true;
  }

  template <class _InitialState, class _ExpectedState, typename... _Inputs>
  bool test(_Inputs... inputs) {
    return test(fsm::state_type_hash_v<_InitialState>,
                fsm::state_type_hash_v<_ExpectedState>, inputs...);
  }
};
} // namespace fsm::validation