#pragma once
#include <algorithm>
#include <cstddef>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

using state_hash_t = std::size_t;

template <typename _InputType, typename _OutputType> class State {
public:
  using input_t = _InputType;
  using output_t = _OutputType;
  using output_state_pair_t = std::pair<_OutputType, state_hash_t>;

public:
  virtual ~State() {}

  virtual output_state_pair_t operator()() = 0;
  virtual output_state_pair_t operator()(_InputType) = 0;
};

template <class _T>
const static inline state_hash_t state_hash_v =
    typeid(std::decay_t<_T> *).hash_code();

template <class _T> static inline state_hash_t get_type_hash(_T &) {
  return state_hash_v<_T>;
}

template <typename _InputType, typename _OutputType, class _State,
          class... _States>
class FiniteStateMachine {
  friend class DebugFiniteStateMachine;

public:
  using state_input_t = _InputType;
  using state_output_t = _OutputType;
  using state_base_t = State<_InputType, _OutputType>;
  using state_tuple_t = std::tuple<_State, _States...>;
  using state_map_t =
      std::unordered_map<state_hash_t, std::unique_ptr<state_base_t>>;
  using state_map_iterator_t = typename state_map_t::iterator;

  static constexpr size_t state_count = 1 + sizeof...(_States);

private:
  state_map_t _states;
  state_map_iterator_t _current_state_it;
  state_hash_t _current_state_hash;
  std::vector<state_hash_t> _acceptance_states;

public:
  FiniteStateMachine()
      : _states(state_count), _current_state_it(_states.end()) {
    _states.emplace(
        std::make_pair(state_hash_v<_State>, std::make_unique<_State>()));

    _states.emplace(
        std::make_pair(state_hash_v<_States>, std::make_unique<_States>())...);

    _transition(state_hash_v<_State>);
    if (_current_state_it == _states.end()) {
      throw std::runtime_error("Failed to set initial state");
    }
  }

  FiniteStateMachine &reset() {
    *this = FiniteStateMachine();
    return *this;
  }

  _OutputType step() {
    auto &current_state = _get_current_state();
    auto [output, next] = current_state();
    _transition(next);
    return output;
  }

  _OutputType step(_InputType input) {
    auto &current_state = _get_current_state();
    auto [output, next] = current_state(input);
    _transition(next);
    return output;
  }

  template <class... RemainingInputType>
  _OutputType transduce(_InputType input, RemainingInputType... inputs) {
    step(input);
    return transduce(inputs...);
  }

  _OutputType transduce(size_t count = 1) {
    if (count == 0) {
      return step();
    } else {
      step();
      return transduce(count - 1);
    }
  }

  template <class State> bool is_acceptance_state() {
    return std::find(_acceptance_states.begin(), _acceptance_states.end(),
                     state_hash_v<State>) != _acceptance_states.end();
  }

  template <class State, class... States> void register_acceptance_state() {
    static_assert(std::is_same_v<State, _State> ||
                  std::is_same_v<State, _States...>);
    if (!is_acceptance_state<State>()) {
      _acceptance_states.push_back(state_hash_v<State>);
    }

    register_acceptance_state<States...>();
  }

  template <class State, class... States> void deregister_acceptance_state() {
    static_assert(std::is_same_v<State, _State> ||
                  std::is_same_v<State, _States...>);
    if (is_acceptance_state<State>()) {
      std::remove(_acceptance_states.begin(), _acceptance_states.end(),
                  state_hash_v<State>);
    }

    deregister_acceptance_state<States...>();
  }

  template <class State> bool is_current_state() const {
    return _current_state_it == _states.find(state_hash_v<State>);
  }

  state_hash_t current_state_hash() const { return _current_state_hash; }

  bool is_current_state_valid() const {
    return _current_state_it != _states.end();
  }

protected:
  state_base_t &_get_current_state() {
    if (!is_current_state_valid()) {
      throw std::runtime_error("Current state is invalid.");
    }
    return *_current_state_it->second;
  }

  void _transition(size_t next) {
    _current_state_it = _states.find(next);
    if (!is_current_state_valid()) {
      _current_state_hash = 0;
    } else {
      _current_state_hash = next;
    }
  }
};

class DebugFiniteStateMachine {
public:
  template <class _Fsm>
  static state_hash_t current_state_hash(const _Fsm &automaton) {
    return get_type_hash(automaton._current_state);
  }

  template <class _Fsm> static std::string state_info(const _Fsm &automaton) {
    std::ostringstream os;
    os << "State,State Hash\n";
    std::for_each(automaton._states.begin(), automaton._states.end(),
                  [&os](auto &state_mapping) {
                    os << typeid(state_mapping.second).name() << ","
                       << state_mapping.first << "\n";
                  });
    return os.str();
  }

  template <class _Fsm>
  static std::string current_state_info(const _Fsm &automaton) {
    std::ostringstream os;
    // os << "Current State,State Hash\n" +
    //           std::string(typeid(automaton._get_current_state()).name()) +
    //           "," + std::to_string(current_state_hash(automaton)) + "\n";
    return os.str();
  }
};

template <class _Fsm0, class _Fsm1> struct is_cascadable {
  static constexpr bool value = std::is_same_v<typename _Fsm0::state_output_t,
                                               typename _Fsm1::state_input_t>;
};

// template <class _Fsm0, class _Fsm1>
// constexpr bool is_cascadable_v = is_cascadable<_Fsm0, _Fsm1>::value;

template <class _Fsm0, class _Fsm1, class... _Fsmn>
constexpr bool is_cascadable_v = is_cascadable<_Fsm0, _Fsm1>::value;
/*
`CascadingFiniteStateMachine` composes of 2 or more finite state machines
in which they are connected where the output of the one is the input to
the next.
*/
template <class _Fsm0, class _Fsm1, class... _Fsmn>
class CascadingFiniteStateMachine {
  static_assert(is_cascadable_v<_Fsm0, _Fsm1, _Fsmn...>);

public:
  static constexpr size_t fsm_count = 2 + sizeof...(_Fsmn);

  using fsm_tuple_t = std::tuple<_Fsm0 &, _Fsm1 &, _Fsmn &...>;
  using input_t = typename _Fsm0::state_input_t;
  using output_t = typename std::decay_t<
      std::tuple_element_t<fsm_count - 1, fsm_tuple_t>>::state_output_t;

protected:
  fsm_tuple_t _fsms;

public:
  CascadingFiniteStateMachine(_Fsm0 &fsm0, _Fsm1 &fsm1, _Fsmn &...fsmn)
      : _fsms(fsm_tuple_t(fsm0, fsm1, fsmn...)) {}

  output_t step() {
    auto out = std::get<0>(_fsms).step();
    return _cascade_step(out, 1);
  }

  output_t step(input_t input) {
    auto out = std::get<0>(_fsms).step(input);
    return _cascade_step(out, 1);
  }

protected:
  template <typename FsmOutput>
  output_t _cascade_step(FsmOutput out, const size_t index = 1) {
    if (index < fsm_count - 1) {
      auto next_out = std::get<index>(_fsms).step(out);
      return _cascade_step(index + 1, next_out);
    } else {
      return std::get<index>(_fsms).step(out);
    }
  }
};