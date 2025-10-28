#include "../../include/FiniteStateMachine.hpp"
#include <cassert>

class IncrementState;
class CheckState;
class FinalState;

class IncrementState : public fsm::non_terminal_state_t<int> {
public:
  IncrementState(const shared_ptr_t &s) : fsm::non_terminal_state_t<int>(s) {}

  size_t process() {
    get_object() += 1;
    return fsm::state_type_hash_v<CheckState>;
  }
};

class CheckState : public fsm::non_terminal_state_t<int> {
public:
  CheckState(const shared_ptr_t &s) : fsm::non_terminal_state_t<int>(s) {}
  size_t process() {
    if (get_object() < 100) {
      return fsm::state_type_hash_v<IncrementState>;
    } else {
      return fsm::state_type_hash_v<FinalState>;
    }
  }
};

class FinalState : public fsm::terminal_state_t<int> {
public:
  FinalState(const shared_ptr_t &s) : fsm::terminal_state_t<int>(s) {}
};

using incrementer_fsm_t =
    fsm::FiniteStateMachine<int, IncrementState, CheckState, FinalState>;