#include "../../include/FiniteStateMachine.hpp"
#include <cassert>
#include <cstddef>

namespace EvenZerosExample {
class EvenState;
class OddState;

class EvenState : public fsm::final_state_t<std::nullptr_t> {
public:
  EvenState(const shared_ptr_t &s) : fsm::final_state_t<std::nullptr_t>(s) {}
  // TODO change parameter to template arguments that check if the value is
  // convertible to bool.
  size_t process(bool is_zero) {
    if (!is_zero) {
      return fsm::state_type_hash_v<EvenState>;
    } else {
      return fsm::state_type_hash_v<OddState>;
    }
  }
};

class OddState : public fsm::state_t<std::nullptr_t> {
public:
  OddState(const shared_ptr_t &s) : fsm::state_t<std::nullptr_t>(s) {}
  size_t process(bool is_zero) {
    if (!is_zero) {
      return fsm::state_type_hash_v<OddState>;
    } else {
      return fsm::state_type_hash_v<EvenState>;
    }
  }
};

using even_zeros_acceptor_t = fsm::Acceptor<EvenState, OddState>;
}; // namespace EvenZerosExample