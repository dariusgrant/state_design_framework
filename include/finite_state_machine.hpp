#pragma once
#include "state.hpp"
#include "state_transition.hpp"
#include <any>
#include <memory>
#include <stdexcept>
#include <unordered_set>
#include <vector>

/*

*/
template <class obj_t> class finite_state_machine {
public:
  friend class state<obj_t>;

private:
  std::shared_ptr<obj_t> _object;
  std::unordered_set<std::shared_ptr<state<obj_t>>> _states;
  std::weak_ptr<state<obj_t>> _current_state;
  std::vector<state_transition<obj_t>> _transitions;

public:
  finite_state_machine<obj_t>(obj_t object,
                              std::initializer_list<state<obj_t>> states,
                              std::string initial_state)
      : _object(std::make_shared<obj_t>(object)) {

    for (auto s : states) {
      _states.insert(std::make_shared<state<obj_t>>(s));
    }

    if (!_get_state(initial_state)) {
      throw std::runtime_error("The specified initial state \"" +
                               initial_state +
                               "\" is not within the set of states.\n");
    }
    _set_state(initial_state, "Initial State\n");
  };

  std::shared_ptr<obj_t> get_object() { return _object; }
  void process(std::any input) { _current_state->process(this, input); }

private:
  std::shared_ptr<state<obj_t>> _get_state(std::string state_name) {
    if (!_states.contains(state_name)) {
      return std::shared_ptr<state<obj_t>>();
    }
    return std::make_shared<state<obj_t>>(_states[state_name]);
  }

  void _set_state(std::string state_name, std::string reason) {
    auto next_state = _get_state(state_name);
    if (next_state) {
      throw std::runtime_error("State \"" + state_name +
                               "\" does not exist within the machine.\n");
    }

    _transitions.push_back(
        state_transition<obj_t>(_current_state->_name, next_state, reason));
    _current_state = next_state.get();
  }
};
