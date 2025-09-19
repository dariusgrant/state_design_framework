#include "../../include/FiniteStateMachine.hpp"
#include <cstdint>
#include <random>

class PollState : public fsm::NonTerminalState<uint64_t> {
private:
  std::random_device rd; // No copyable - thus the rule of 3 is applied here
  std::mt19937 gen;
  std::uniform_int_distribution<uint64_t> dist;

public:
  PollState(const PollState::shared_ptr_t &obj)
      : fsm::NonTerminalState<obj_t>(obj), rd(), gen(rd()),
        dist(0, UINT64_MAX) {}
  PollState(const PollState &a) : fsm::NonTerminalState<obj_t>(a) { *this = a; }
  PollState &operator=(const PollState &a) {
    this->_obj = a._obj;
    this->gen = a.gen;
    this->dist = a.dist;
    return *this;
  }

public:
  STATE_EXIT_FUNCTION_NO_OP

  void enter() { *_obj.lock() = dist(gen); }

  auto transition() { return fsm::StateIdentity<PollState>(); };
};

using SimPacketPoller = fsm::FiniteStateMachine<uint64_t, PollState>;
