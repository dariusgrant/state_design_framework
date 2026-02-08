#include "FiniteStateMachine.hpp"
#include <cassert>
#include <chrono>
#include <exception>
#include <functional>
#include <iostream>

enum class LightState {
  UNKNOWN,
  OFF,
  RED,
  YELLOW,
  GREEN,
};

struct LightInput {
  bool started;
  std::chrono::high_resolution_clock::time_point end_time;
};

class LightStateNode : public StateNode<LightInput, LightState> {
public:
  const LightState state;

  LightStateNode(std::string name, LightState state)
      : StateNode(name), state(state) {}

  void enter(LightInput *) override {
    std::cout << "Entering " << name << " State\n";
  }

  void exit(LightInput *) override {
    std::cout << "Exiting " << name << " State\n";
  }

  StateTransition process(LightInput *input) override {
    if (!input) {
      std::cout << "Received nullptr\n";
      return state;
    }

    return _process_impl(input);
  }

  virtual StateTransition _process_impl(LightInput *input) = 0;

  bool reached_end_time(
      std::chrono::high_resolution_clock::time_point end_time) const {
    return std::chrono::high_resolution_clock::now() >= end_time;
  }

  StateTransition transition_on_endtime(LightInput *input, std::string state) {
    if (input->end_time.time_since_epoch().count() == 0) {
      std::cout << "No end time set\n";
      return this->state;
    } else if (!reached_end_time(input->end_time)) {
      std::cout << "Waiting "
                << (input->end_time - std::chrono::high_resolution_clock::now())
                       .count()
                << "seconds...\n";
      return this->state;
    } else {
      auto next = get_child<LightStateNode>(state);
      return {next->state, next};
    }
  }
};

class OffState : public LightStateNode {
public:
  OffState() : LightStateNode("Off", LightState::OFF) {}

  StateTransition _process_impl(LightInput *input) override {
    if (input->started) {
      std::cout << "Starting Race Light!\n";
      auto next = get_child<LightStateNode>("Red");
      return {next->state, next};
    } else {
      return state;
    }
  }
};

class RedState : public LightStateNode {
public:
  RedState() : LightStateNode("Red", LightState::RED) {}

  StateTransition _process_impl(LightInput *input) override {
    return transition_on_endtime(input, "Yellow");
  }
};

class YellowState : public LightStateNode {
public:
  YellowState() : LightStateNode("Yellow", LightState::YELLOW) {}

  StateTransition _process_impl(LightInput *input) override {
    return transition_on_endtime(input, "Green");
  }
};

class GreenState : public LightStateNode {
public:
  GreenState() : LightStateNode("Green", LightState::GREEN) {}

  StateTransition _process_impl(LightInput *input) override {
    if (input->started) {
      auto next = get_child<LightStateNode>("Red");
      return {next->state, next};
    }
    return transition_on_endtime(input, "Off");
  }
};

template <class _FSM> struct Expected {
  bool exception_occurrence = false;
  std::function<bool(const _FSM *)> checker = [](auto) { return true; };
};

template <typename _FSM, typename... _Args>
void test_fsm_construction(Expected<_FSM> expected, _Args... args) {
  std::cout << "Test FSM Construction\n";
  bool exception_occurred = false;
  _FSM *fsm;
  try {
    fsm = new _FSM(args...);
  } catch (...) {
    exception_occurred = true;
  }

  assert(expected.exception_occurrence == exception_occurred);
  if (expected.checker) {
    assert(expected.checker(fsm));
  }
}

int main() {
  auto off = OffState();
  auto red = RedState();
  auto yellow = YellowState();
  auto green = GreenState();

  off.connect(&red);
  red.connect(&yellow);
  yellow.connect(&green);
  green.connect(&off);

  using fsm_t = FiniteStateMachine<LightInput, LightState>;
  Expected<fsm_t> expected{true};
  test_fsm_construction(expected, nullptr);

  expected.exception_occurrence = false;
  expected.checker = [&](const fsm_t *fsm) {
    return fsm->current_state() == &off;
  };
  test_fsm_construction(expected, &off);

  // FSM()
  auto fsm = FiniteStateMachine(&off);
  assert(fsm.current_state() == &off);
  assert(fsm.transduce() == LightState::OFF);

  auto input = LightInput();
  assert(fsm.transduce(&input) == LightState::OFF);
}