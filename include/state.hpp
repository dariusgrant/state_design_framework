#pragma once

#include <any>
#include <functional>
#include <stdexcept>
#include <string>

template <class obj_t>
inline constexpr auto throw_on_access_not_implemented(std::string state_name, std::string access_name) {
  return [=](obj_t &obj) {
    throw std::runtime_error(
        "[" + state_name + "_State] No implementation for `state::_on_" + access_name + "_func`");
  };
}

template <class fsm_t>
inline constexpr auto
throw_process_not_implemented(std::string state_name) {
  return [=](fsm_t &fsm, std::any input) {
    throw std::runtime_error("[" + state_name +
                             " State] No implementation for `state::_process_func`");
  };
}
template <class obj_t> class finite_state_machine;

template <class obj_t> using on_access_func = std::function<void(obj_t &obj)>;

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
  on_access_func<obj_t> _on_enter_func;
  on_access_func<obj_t> _on_exit_func;
  state_proc_func _process_func;

public:
  state(std::string name)
      : _name(name),
        _on_enter_func(throw_on_access_not_implemented<obj_t>(name, "enter")),
        _on_exit_func(throw_on_access_not_implemented<obj_t>(name, "exit")),
        _process_func(throw_process_not_implemented<finite_state_machine<obj_t>>(name)) {}

  // TODO: Modify constructor to include access functions.
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

  void on_enter(obj_t &obj) { _on_enter_func(obj); }

  void on_exit(obj_t &obj) { _on_exit_func(obj); }

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
  null_state() : state<obj_t>("Null") {};
};
