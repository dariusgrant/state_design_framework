#include "../include/finite_state_machine.hpp"
#include <any>
#include <iostream>
#include <stdexcept>

// Step 1: Define all inputs that the fsm can process.
enum class LightButton { p1, p2 };

// Step 2: Define the object the fsm will be manipulating
// by listing only it's attributes and behavior (only when
// acting on itself or another object - do not define an
// interface for another object to manipulate.)
class Light {
private:
  // Information about itself.
  std::string _name;
  bool _illuminated;

public:
  Light(std::string name) : _name(name), _illuminated(false) {}

  // Behaviors on what it can do. A light can illuminate and darken.
  void illuminate() { _illuminated = true; }
  void darken() { _illuminated = false; }

  // Any behaviors that can be observed outside of
  // the object should be made available, such as a
  // lightbulb's illumination. On the contrary,
  // the electrical charge that's sent to the intial
  // point of bulb before illumination cannot normally
  // be observed externally. So that wouldn't be a
  // public observation.
  bool is_illuminated() { return _illuminated; }
};

// Step 3: Define the states.
class light_state : public state<Light> {
public:
  light_state(std::string name) : state<Light>(name) {}

  void on_enter(Light &light) { light.illuminate(); }

  void on_exit(Light &light) { light.darken(); }

  void process(finite_state_machine<Light> &fsm, std::any input) {

    try {
      auto button = std::any_cast<LightButton>(input);
      if (_name == "none") {
        fsm.set_state("l1");
      } else if (_name == "l1") {
        auto next_state = button == LightButton::p1 ? "l2" : "l3";
        fsm.set_state(next_state);
      } else {
        fsm.set_state("none");
      }
    } catch (std::bad_any_cast &e) {
      std::cout << "Ignore bad input.\n";
    }
  }
};
