#include "FiniteStateMachine.hpp"
#include <cassert>
#include <chrono>
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

class LightStateNode : public StateNode<LightState> {
public:
  const LightState state;

  LightStateNode(std::string name, LightState state)
      : StateNode(name), state(state) {}

  StateTransition process(void *input) override {
    std::cout << "Entering " << name << " State\n";
    if (!input) {
      std::cout << "Received nullptr - noop\n";
      return StateTransition(state, nullptr);
    }

    return _process_impl(static_cast<LightInput *>(input));
  }

  virtual StateTransition _process_impl(LightInput *input) = 0;

  bool reached_end_time(
      std::chrono::high_resolution_clock::time_point end_time) const {
    return std::chrono::high_resolution_clock::now() >= end_time;
  }

  LightStateNode *get_light_state_node(std::string state) {
    return static_cast<LightStateNode *>(get_child(state));
  }

  StateTransition transition_on_endtime(LightInput *input, std::string state) {
    if (input->end_time.time_since_epoch().count() == 0) {
      std::cout << "No end time set\n";
      return StateTransition(this->state, this);
    } else if (!reached_end_time(input->end_time)) {
      std::cout << "Waiting "
                << (input->end_time - std::chrono::high_resolution_clock::now())
                       .count()
                << "seconds...\n";
      return StateTransition(this->state, this);
    } else {
      auto next = get_light_state_node(state);
      return StateTransition(next->state, next);
    }
  }
};

class OffState : public LightStateNode {
public:
  OffState() : LightStateNode("Off", LightState::OFF) {}

  StateTransition _process_impl(LightInput *input) override {
    if (input->started) {
      std::cout << "Starting Race Light!\n";
      auto next = get_light_state_node("Red");
      return StateTransition(next->state, next);
    }
    return StateTransition(state, nullptr);
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
      auto next = get_light_state_node("Red");
      return StateTransition(next->state, next);
    }
    return transition_on_endtime(input, "Off");
  }
};

int main() {
  auto off = OffState();
  auto red = RedState();
  auto yellow = YellowState();
  auto green = GreenState();

  off.connect(&red);

  auto input = LightInput();
  auto fsm = FiniteStateMachine(&off);

  assert(fsm.transduce(input) == LightState::OFF);
}