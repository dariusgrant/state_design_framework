#include "../../include/FiniteStateMachine.hpp"
#include <cassert>

class EvenState;
class OddState;

class EvenState : public fsm::final_state_t<class Obj><int> {
public:
  IncrementState(const shared_ptr_t &s) : fsm::state_t<int>(s) {}

  size_t process() {
    get_object() += 1;
    return fsm::state_type_hash_v<CheckState>;
  }
};

class CheckState : public fsm::state_t<int> {
public:
  CheckState(const shared_ptr_t &s) : fsm::state_t<int>(s) {}
  size_t process() {
    if (get_object() < 100) {
      return fsm::state_type_hash_v<IncrementState>;
    } else {
      return fsm::state_type_hash_v<FinalState>;
    }
  }
};

class FinalState : public fsm::final_state_t<int> {
public:
  FinalState(const shared_ptr_t &s) : fsm::final_state_t<int>(s) {}
};

using incrementer_fsm_t =
    fsm::FiniteStateMachine<int, IncrementState, CheckState, FinalState>;