#include "../../include/FiniteStateMachine.hpp"
#include <cassert>

class LockedState;
class UnlockedState;
class FinalState;

class LockedState : public fsm::non_terminal_state_t<int> {
public:
  void enter() { get_object() += 1; }

  size_t transition() { return fsm::state_type_hash_v<UnlockedState>; }
};

class UnlockedState : public fsm::non_terminal_state_t<int> {
public:
  size_t transition() {
    if (get_object() < 100) {
      return fsm::state_type_hash_v<LockedState>;
    } else {
      return fsm::state_type_hash_v<FinalState>;
    }
  }
};

class FinalState : public fsm::terminal_state_t<int> {};

using incrementer_fsm_t =
    fsm::FiniteStateMachine<int, LockedState, UnlockedState, FinalState>;