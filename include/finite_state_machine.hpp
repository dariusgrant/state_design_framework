#pragma once
#include "state.hpp"
#include "state_transition.hpp"
#include <any>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

/*

*/
template <class obj_t> class finite_state_machine {
  using state_map =
      std::unordered_map<std::string, std::shared_ptr<state<obj_t>>,
                         typename state<obj_t>::hash,
                         typename state<obj_t>::equal>;

private:
  inline static const std::shared_ptr<null_state<obj_t>> _null_state =
      std::make_shared<null_state<obj_t>>(null_state<obj_t>());
  std::shared_ptr<obj_t> _object;
  state_map _states;
  std::weak_ptr<state<obj_t>> _current_state;
  std::vector<state_transition<obj_t>> _transitions;

public:
  finite_state_machine(obj_t object, std::initializer_list<state<obj_t>> states,
                       std::string initial_state)
      : _object(std::make_shared<obj_t>(object)), _current_state(_null_state) {
    for (auto s : states) {
      _states[s.get_name()] = std::make_shared<state<obj_t>>(s);
    }
    if (!_get_state(initial_state)) {
      throw std::runtime_error("The specified initial state \"" +
                               initial_state +
                               "\" is not within the set of states.\n");
    }
    _set_state(initial_state, "Initial State\n");
  };

  // constexpr null_state<obj_t>&

  std::shared_ptr<obj_t> get_object() { return _object; }

  void process(std::any input) { _current_state.lock()->process(*this, input); }

private:
  std::shared_ptr<state<obj_t>> _get_state(std::string state_name) {
    if (!_states.contains(state_name)) {
      return std::shared_ptr<state<obj_t>>();
    }
    return _states[state_name];
  }

  void _set_state(std::string state_name, std::string reason) {
    auto next_state = _get_state(state_name);
    if (!next_state) {
      throw std::runtime_error("State \"" + state_name +
                               "\" does not exist within the machine.\n");
    }

    _transitions.push_back(state_transition<obj_t>(
        _current_state.lock()->get_name(), next_state->get_name(), reason));
    _current_state = next_state;
  }
};