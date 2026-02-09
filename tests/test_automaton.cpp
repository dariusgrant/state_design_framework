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
  UNKNOWN,
  OFF,
  RED,
  YELLOW,
  GREEN,
};

class LightInputVariant
    : public InputVariant<bool,
                          std::chrono::high_resolution_clock::time_point> {};

// Get the index of an alternative type.
template <typename _Alternative, size_t N = 0>
constexpr size_t alternative_index() {
  if constexpr (N >= std::variant_size_v<LightInputVariant::variant>) {
    return 0;
  }

  constexpr bool is_index =
      std::is_same_v<_Alternative,
                     std::variant_alternative_t<N, LightInputVariant::variant>>;
  constexpr size_t next = (N + 1 > N ? N : N + 1);
  return is_index ? N : alternative_index<_Alternative, next>();
}

template <typename _Alternative>
constexpr size_t alternative_index_v = alternative_index<_Alternative>();

class LightStateNode : public StateNode<LightInputVariant, LightState> {
public:
  const LightState state;

  LightStateNode(std::string name, LightState state)
      : StateNode(name), state(state) {}

  void enter(const LightInputVariant &input) override {
    std::cout << "Entering " << name << " State\n";
  }

  void exit(const LightInputVariant &input) override {
    std::cout << "Exiting " << name << " State\n";
  }



  StateTransition process(const LightInputVariant &input) override {
    switch (input.index()) {
    case alternative_index_v<std::monostate>:
      std::cout << "Received nullptr\n";
      return state;
    default:
      return _process_impl(input);
    }
  }

  virtual StateTransition _process_impl(const LightInputVariant &input) = 0;

  bool reached_end_time(
      std::chrono::high_resolution_clock::time_point end_time) const {
    return std::chrono::high_resolution_clock::now() >= end_time;
  }

  StateTransition transition_on_endtime(const LightInputVariant &input,
                                        std::string state) {
    auto end_time =
        std::get_if<std::chrono::high_resolution_clock::time_point>(&input);
    if (!end_time) {
      std::cout << "Input is not a `time_point`.\n";
      return this->state;
    } else if (end_time->time_since_epoch().count() == 0) {
      std::cout << "No end time set\n";
      return this->state;
    } else if (!reached_end_time(*end_time)) {
      std::cout
          << "Waiting "
          << (*end_time - std::chrono::high_resolution_clock::now()).count()
          << "seconds...\n";
      return this->state;
    } else {
      auto next = get_child<LightStateNode>(state);
      return {next->state, next};
    }
  }
};

class OffState :
      public
        LightStateNode{
          public : OffState() : LightStateNode("Off", LightState::OFF){}

          StateTransition _process_impl(const LightInputVariant &input)
              override{auto started = std::get_if<bool>(&input);

        if (started == nullptr) {
          std::cout << "Input is not `bool`.\n";
          return state;
        } else if (!(*started)) {
          std::cout << "Race Light not started.\n";
          return state;
        } else {
          std::cout << "Starting Race Light!\n";
          auto next = get_child<LightStateNode>("Red");
          return {next->state, next};
        }
  }
};

class RedState : public LightStateNode {
public:
  RedState() : LightStateNode("Red", LightState::RED) {}

  StateTransition _process_impl(const LightInputVariant &input) override {
    return transition_on_endtime(input, "Yellow");
  }
};

class YellowState : public LightStateNode {
public:
  YellowState() : LightStateNode("Yellow", LightState::YELLOW) {}

  StateTransition _process_impl(const LightInputVariant &input) override {
    return transition_on_endtime(input, "Green");
  }
};

class GreenState : public LightStateNode {
public:
  GreenState() : LightStateNode("Green", LightState::GREEN) {}

  StateTransition _process_impl(const LightInputVariant &input) override {
    auto started = std::get_if<bool>(&input);
    if (started == nullptr) {
      std::cout << "Input is not `bool`.\n";
      return state;
    } else if (*started) {
      auto next = get_child<LightStateNode>("Red");
      return {next->state, next};
    } else {
      return transition_on_endtime(input, "Off");
    }
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

  using fsm_t = FiniteStateMachine<LightInputVariant, LightState>;
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

  auto input = LightInputVariant();
  assert(fsm.transduce(input) == LightState::OFF);
}