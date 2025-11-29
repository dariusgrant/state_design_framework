// Reference:
// https://www.researchgate.net/figure/Negotiation-Protocol-State-Machine_fig1_221549501

#include "../../include/FiniteStateMachine.hpp"
#include <cstddef>
#include <type_traits>

namespace NegotiationProtocol {
class StartState;
class InitiateState;
class ExpectState;
class ServiceState;
class ProcessState;
class StopState;

enum class Input { trigger, request, off, pol, terminate };

class StartState : public fsm::state_t<std::nullptr_t> {
public:
  StartState(const shared_ptr_t &s) : fsm::state_t<std::nullptr_t>(s) {}
  size_t process(Input i) {
    switch (i) {
    case Input::trigger:
      return fsm::state_type_hash_v<InitiateState>;
    case Input::request:
      return fsm::state_type_hash_v<ServiceState>;
    case Input::off:
    case Input::pol:
      return fsm::state_type_hash_v<ServiceState>;
    default:
      return fsm::state_type_hash_v<StartState>;
    }
  }
};

class InitiateState : public fsm::state_t<std::nullptr_t> {
public:
  InitiateState(const shared_ptr_t &s) : fsm::state_t<std::nullptr_t>(s) {}
  size_t process(Input i) {
    switch (i) {
    case Input::request:
      return fsm::state_type_hash_v<ExpectState>;
    default:
      return fsm::state_type_hash_v<InitiateState>;
    }
  }
};

class ExpectState : public fsm::state_t<std::nullptr_t> {
public:
  ExpectState(const shared_ptr_t &s) : fsm::state_t<std::nullptr_t>(s) {}
  size_t process(Input i) {
    switch (i) {
    case Input::request:
      return fsm::state_type_hash_v<ServiceState>;
    case Input::terminate:
      return fsm::state_type_hash_v<StopState>;
    case Input::off:
    case Input::pol:
      return fsm::state_type_hash_v<ProcessState>;
    default:
      return fsm::state_type_hash_v<ExpectState>;
    }
  }
};

class ServiceState : public fsm::state_t<std::nullptr_t> {
public:
  ServiceState(const shared_ptr_t &s) : fsm::state_t<std::nullptr_t>(s) {}
  size_t process(Input i) {
    switch (i) {
    case Input::request:
    case Input::off:
    case Input::pol:
      return fsm::state_type_hash_v<ExpectState>;
    case Input::terminate:
      return fsm::state_type_hash_v<StopState>;
    default:
      return fsm::state_type_hash_v<ServiceState>;
    }
  }
};

class ProcessState : public fsm::state_t<std::nullptr_t> {
public:
  ProcessState(const shared_ptr_t &s) : fsm::state_t<std::nullptr_t>(s) {}
  size_t process(Input i) {
    switch (i) {
    case Input::request:
    case Input::off:
    case Input::pol:
      return fsm::state_type_hash_v<ExpectState>;
    case Input::terminate:
      return fsm::state_type_hash_v<StopState>;
    default:
      return fsm::state_type_hash_v<ServiceState>;
    }
  }
};

class StopState : public fsm::final_state_t<std::nullptr_t> {
public:
  StopState(const shared_ptr_t &s) : fsm::final_state_t<std::nullptr_t>(s) {}
};

using negotiation_protocol_fsm_t =
    fsm::FiniteStateMachine<std::nullptr_t, StartState, InitiateState,
                            ExpectState, ServiceState, ProcessState, StopState>;
}; // namespace NegotiationProtocol