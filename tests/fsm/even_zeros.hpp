#include "../../include/FiniteStateMachine.hpp"

namespace EvenZerosExample {
class EvenState;
class OddState;

class EvenState : public fsm::final_state_t<std::nullptr_t> {
public:
  EvenState(const shared_ptr_t &s) : fsm::final_state_t<std::nullptr_t>(s) {}
  template <class _Int> size_t process(_Int arg) {
    static_assert(std::is_integral_v<_Int>);
    if (arg != 0) {
      return fsm::state_type_hash_v<EvenState>;
    } else {
      return fsm::state_type_hash_v<OddState>;
    }
  }
};

class OddState : public fsm::state_t<std::nullptr_t> {
public:
  OddState(const shared_ptr_t &s) : fsm::state_t<std::nullptr_t>(s) {}
  template <class _Int> size_t process(_Int arg) {
    static_assert(std::is_integral_v<_Int>);
    if (arg != 0) {
      return fsm::state_type_hash_v<OddState>;
    } else {
      return fsm::state_type_hash_v<EvenState>;
    }
  }
};

using even_zeros_acceptor_t = fsm::Acceptor<EvenState, OddState>;
}; // namespace EvenZerosExample