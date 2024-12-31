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
  state<obj_t>(std::string name) : _name(name) {}

  std::string get_name() const { return _name; }
  /*
  What should happened when the State is entered/started.
    input: Input of any type.
  */
  virtual void process(finite_state_machine<obj_t>* fsm, std::any input) {
    throw std::runtime_error("Function `state::process()` not implemented for state \"" + _name + "\".\n");
  };
};

template <class obj_t> struct std::hash<state<obj_t>> {
  std::size_t operator()(const state<obj_t> &s) const {
    return std::hash<std::string>{}(s.get_name());
  }
};
