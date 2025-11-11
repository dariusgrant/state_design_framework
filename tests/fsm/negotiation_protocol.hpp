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

namespace Validaton {
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
    if (_fsm.get_current_state_type_hash() != initial_state) {
      return false;
    }

    _fsm.process(inputs...);

    if (_fsm.get_current_state_type_hash() != next_state) {
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

class TruthTable {
  // CurrentState + Input = NextState
};
}; // namespace Validaton
}; // namespace NegotiationProtocol