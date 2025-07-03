#include "../include/FiniteStateMachine.hpp"
#include <iostream>

/***************************************************
Step 1 - Define the object that will be manipulated.
`StopLight` has the operational ability to change
to a singular color at a time - red, yellow, or
green. When viewing the perspective of reality,
there is some external mechanism that dictates when
the color changes and to which color. The stoplight
itself provides the interface to invoke its
behaviours, in which a finite state machine will be
the driver for.
***************************************************/
class StopLight {
public:
  enum class LightColor { RED, YELLOW, GREEN };

private:
  LightColor _current_color;

public:
  void change_color(LightColor color) { _current_color = color; }

  std::string color_str() const {
    switch (_current_color) {
    case LightColor::RED:
      return "RED";
    case LightColor::YELLOW:
      return "YELLOW";
    default:
      return "GREEN";
    }
  }
};

class LightState : public fsm::AbstractState {
private:
  StopLight::LightColor _color;

public:
  LightState(StopLight::LightColor color) : _color(color) {}

  void enter(StopLight &stop_light, int a) {
    stop_light.change_color(_color);
    std::cout << "Entered " << stop_light.color_str() << " state\n";
  }

  void exit(const StopLight &stop_light, int a) {
    std::cout << "Leaving " << stop_light.color_str() << " state\n";
  }
};

class RedLightState;
class YellowLightState;
class GreenLightState;

class RedLightState : public LightState {
public:
  RedLightState() : LightState(StopLight::LightColor::RED){};

  template <class... StateTypes>
  std::variant<StateTypes...> &
  transition(const StopLight &stop_light, int a,
             fsm::StateMap<StateTypes...> &states) {
    return states[fsm::GetStateTypeIndex<GreenLightState>()];
  }
};

class YellowLightState : public LightState {
public:
  YellowLightState() : LightState(StopLight::LightColor::YELLOW){};

  template <class... StateTypes>
  std::variant<StateTypes...> &
  transition(const StopLight &stop_light, int a,
             fsm::StateMap<StateTypes...> &states) {
    return states[fsm::GetStateTypeIndex<RedLightState>()];
  }
};

class GreenLightState : public LightState {
public:
  GreenLightState() : LightState(StopLight::LightColor::GREEN){};

  template <class... StateTypes>
  std::variant<StateTypes...> &
  transition(const StopLight &stop_light, int a,
             fsm::StateMap<StateTypes...> &states) {
    return states[fsm::GetStateTypeIndex<YellowLightState>()];
  }
};

class NonStateClass {};

int main() {

  auto stop_light_fsm =
      fsm::FiniteStateMachine<StopLight, RedLightState, YellowLightState,
                              GreenLightState>();
  stop_light_fsm.start(0);
  stop_light_fsm.process(0, 0);
  stop_light_fsm.process(0, 0);
}