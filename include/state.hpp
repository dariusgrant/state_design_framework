#pragma once

#include <any>
#include <stdexcept>
#include <string>

template <class obj_t> class finite_state_machine;
/*
An abstract represention of a state of a finite state machine.

  obj_t: The type of object the state will be operated on.
*/
template <class obj_t> class state {
private:
  std::string _name;

public:
  state(std::string name) : _name(name) {}

  struct hash {
    std::size_t operator()(const state<obj_t> &s) const {
      return std::hash<std::string>{}(s._name);
    }
  };

  struct equal {
    size_t operator()(const state<obj_t> &a, const state<obj_t> &b) const {
      return a._name == b._name;
    }
  };

  std::string get_name() const { return _name; }
  /*
  What should happened when the State is entered/started.
    input: Input of any type.
  */
  virtual void process(finite_state_machine<obj_t> &fsm, std::any input) {
    throw std::runtime_error(
        "Function `state::process()` not implemented for state \"" + _name +
        "\".\n");
  };
};
