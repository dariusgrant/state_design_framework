#include "FiniteStateMachine.hpp"
#include <cassert>
#include <chrono>
#include <cstdint>
#include <exception>
#include <functional>
#include <iostream>
#include <utility>
#include <variant>

enum class LightState {
  OFF,
  RED,
  YELLOW,
  GREEN,
};

class LightInputVariant
    : public InputVariant<bool,
                          std::chrono::high_resolution_clock::time_point> {};

class LightStateNode : public StateNode<LightInputVariant, LightState> {
public:
  const LightState state;
  LightStateNode(std::string name, LightState state)
      : StateNode(name), state(state) {}

  void enter(const LightInputVariant &) override {
    std::cout << "Entering " << name << " State\n";
  }

  void exit(const LightInputVariant &) override {
    std::cout << "Exiting " << name << " State\n";
  }

  state_transition_t process(const LightInputVariant &input) override {
    switch (input.index()) {
    case alternative_index_v<LightInputVariant, bool>:
      return _process_impl(std::get<bool>(input));
    case alternative_index_v<LightInputVariant,
                             std::chrono::high_resolution_clock::time_point>:
      return _process_impl(
          std::get<std::chrono::high_resolution_clock::time_point>(input));
    default:
      std::cout << "No transition.\n";
      return {state};
      ;
    }
  }

  virtual state_transition_t _process_impl(const bool &) { return state; }
  virtual state_transition_t
  _process_impl(const std::chrono::high_resolution_clock::time_point &) {
    return state;
  }

  bool reached_end_time(
      const std::chrono::high_resolution_clock::time_point &end_time) const {
    return std::chrono::high_resolution_clock::now() >= end_time;
  }

  state_transition_t transition_on_endtime(
      const std::chrono::high_resolution_clock::time_point &end_time,
      std::string state) {
    if (end_time.time_since_epoch().count() == 0) {
      std::cout << "No end time set.\n";
      return this->state;
    } else if (!reached_end_time(end_time)) {
      std::cout
          << "Waiting "
          << (end_time - std::chrono::high_resolution_clock::now()).count()
          << " seconds...\n";
      return {this->state, nullptr, LightInputVariant{end_time}};
    } else {
      auto next = get_child<LightStateNode>(state);
      return {next->state, next};
    }
  }
};

class OffStateNode : public LightStateNode {
public:
  OffStateNode() : LightStateNode("Off", LightState::OFF) {}

  state_transition_t _process_impl(const bool &started) override {
    if (!started) {
      std::cout << "Race Light not started.\n";
      return state;
    } else {
      std::cout << "Starting Race Light!\n";
      auto next = get_child<LightStateNode>("Red");
      return {next->state, next,
              LightInputVariant{std::chrono::high_resolution_clock::now() +
                                std::chrono::seconds(3)}};
    }
  }
};

class RedStateNode : public LightStateNode {
public:
  RedStateNode() : LightStateNode("Red", LightState::RED) {}

  state_transition_t _process_impl(
      const std::chrono::high_resolution_clock::time_point &end_time) override {
    return transition_on_endtime(end_time, "Yellow");
  }
};

class YellowStateNode : public LightStateNode {
public:
  YellowStateNode() : LightStateNode("Yellow", LightState::YELLOW) {}

  state_transition_t _process_impl(
      const std::chrono::high_resolution_clock::time_point &end_time) override {
    std::cout << "Processing yellow state\n";
    return transition_on_endtime(end_time, "Green");
  }
};

class GreenStateNode : public LightStateNode {
public:
  GreenStateNode() : LightStateNode("Green", LightState::GREEN) {}

  state_transition_t _process_impl(
      const std::chrono::high_resolution_clock::time_point &end_time) override {
    return transition_on_endtime(end_time, "Off");
  }

  state_transition_t _process_impl(const bool &started) override {
    if (started) {
      auto next = get_child<LightStateNode>("Red");
      return {next->state, next};
    } else {
      return state;
    }
  }
};

class RaceLight {
private:
  LightState _light_state;
  OffStateNode _off_state_node;
  RedStateNode _red_state_node;
  YellowStateNode _yellow_state_node;
  GreenStateNode _green_state_node;
  FiniteStateMachine<LightInputVariant, LightState> _fsm;

public:
  RaceLight()
      : _light_state(LightState::OFF), _off_state_node(), _red_state_node(),
        _yellow_state_node(), _green_state_node(), _fsm(&_off_state_node) {
    _off_state_node.connect(&_red_state_node);
    _red_state_node.connect(&_red_state_node);
    _red_state_node.connect(&_yellow_state_node);
    _yellow_state_node.connect(&_green_state_node);
    _green_state_node.connect(&_off_state_node);
  }

  const LightState &light_state() const { return _light_state; }

  void start() { _light_state = _fsm.transduce({true}); }
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
  auto off = OffStateNode();
  auto red = RedStateNode();
  auto yellow = YellowStateNode();
  auto green = GreenStateNode();

  off.connect(&red);
  red.connect(&yellow);
  yellow.connect(&green);
  green.connect(&off);

  // using fsm_t = FiniteStateMachine<LightInputVariant, LightState>;
  // Expected<fsm_t> expected{true};
  // test_fsm_construction(expected, nullptr);

  // expected.exception_occurrence = false;
  // expected.checker = [&](const fsm_t *fsm) {
  //   return fsm->current_state() == &off;
  // };
  // test_fsm_construction(expected, &off);

  // FSM()
  // auto fsm = FiniteStateMachine(&off);
  // assert(fsm.current_state() == &off);
  // assert(fsm.transduce() == LightState::OFF);

  // auto input = LightInputVariant();
  // assert(fsm.transduce(input) == LightState::OFF);

  auto race_light = RaceLight();
  assert(race_light.light_state() == LightState::OFF);
  race_light.start();
  // assert(race_light.light_state() == LightState::RED);
  // race_light.start();
  // assert(race_light.light_state() == LightState::RED);
}