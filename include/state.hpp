#pragma once

#include <any>
#include <functional>
#include <stdexcept>
#include <string>

template <class obj_t> class finite_state_machine;

// A generic process function when operating on an instance of obj_t.
template <class obj_t>
using proc_func =
    std::function<void(finite_state_machine<obj_t> &fsm, std::any input)>;

/*
An abstract represention of a state of a finite state machine.

  obj_t: The type of object the state will be operated on.
*/
template <class obj_t> class state {
public:
  // A state specific process function when operating on an instance of obj_t.
  using state_proc_func = proc_func<obj_t>;

protected:
  std::string _name;
  state_proc_func _process_func;

public:
  state(std::string name)
      : _name(name),
        _process_func([name](finite_state_machine<obj_t> &, std::any input) {
          throw std::runtime_error(
              "No implementation for `state::process_func` for state \"" +
              name + "\"");
        }) {}

  state(std::string name, state_proc_func process_func)
      : _name(name), _process_func(process_func) {}

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
  void process(finite_state_machine<obj_t> &fsm, std::any input) {
    _process_func(fsm, input);
  };
};

template <class state_t, class state_ptr>
state_t::state_proc_func create_state_proc_func(state_ptr state) {
  return std::bind(&state_t::process, state, std::placeholders::_1,
                   std::placeholders::_2);
}

template <class obj_t> class null_state final : public state<obj_t> {
public:
  null_state() : state<obj_t>("Null"){};
};
