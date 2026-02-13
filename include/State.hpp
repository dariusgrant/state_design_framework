#pragma once
#include "utility/StateType.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <variant>

template <typename _InputType, typename _OutputType> class StatePrototype {
public:
  using input_t = _InputType;
  using output_t = _OutputType;
  using output_state_pair_t = std::pair<_OutputType, size_t>;

public:
  // StatePrototype() = default;
  // StatePrototype(StatePrototype &other) = default;
  // StatePrototype(const StatePrototype &other) = default;
  virtual std::pair<_OutputType, size_t> operator()() = 0;
  virtual std::pair<_OutputType, size_t> operator()(_InputType) = 0;

  virtual ~StatePrototype() {}

  // template <class _Other> bool operator==(_Other &state) { return true; }
  // virtual std::pair<_OutputType, size_t> operator()(_InputType input,
  // _MemType memory) = 0;
};

// Intended for mapping a variant's value type to its instance
template <class _T>
const static inline std::size_t type_hash_v =
    typeid(std::decay_t<_T> *).hash_code();

template <class _T> static inline std::size_t get_type_hash(_T &) {
  return type_hash_v<_T>;
}

template <typename _InputType, typename _OutputType, class _State,
          class... _States>
class AutomatonPrototype {
  friend class DebugAutomatonPrototype;

public:
  using state_base_t = StatePrototype<_InputType, _OutputType>;
  using state_tuple_t = std::tuple<_State, _States...>;
  using state_map_t = std::unordered_map<size_t, std::unique_ptr<state_base_t>>;

  static constexpr size_t state_count = 1 + sizeof...(_States);

private:
  state_map_t _states;
  typename state_map_t::iterator _current_state_it;

public:
  AutomatonPrototype()
      : _states(state_count), _current_state_it(_states.end()) {
    _states.emplace(
        std::make_pair(type_hash_v<_State>, std::make_unique<_State>()));

    _states.emplace(
        std::make_pair(type_hash_v<_States>, std::make_unique<_States>())...);

    _current_state_it = _states.find(type_hash_v<_State>);
    if (_current_state_it == _states.end()) {
      throw std::runtime_error("Failed to set initial state");
    }
  }

  _OutputType transduce() {
    auto &current_state = _get_current_state();
    auto [output, next] = current_state();
    _current_state_it = _states.find(next);
    return output;
  }

  _OutputType transduce(_InputType input) {
    auto &current_state = _get_current_state();
    auto [output, next] = current_state(input);
    _current_state_it = _states.find(next);
    return output;
  }

  template <class State> bool is_current_state() const {
    return _current_state_it == _states.find(type_hash_v<State>);
  }

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
};

class DebugAutomatonPrototype {
public:
  template <class _Automaton>
  static size_t current_state_hash(const _Automaton &automaton) {
    return get_type_hash(automaton._current_state);
  }

  template <class _Automaton>
  static std::string state_info(const _Automaton &automaton) {
    std::ostringstream os;
    os << "State,State Hash\n";
    std::for_each(automaton._states.begin(), automaton._states.end(),
                  [&os](auto &state_mapping) {
                    os << typeid(state_mapping.second).name() << ","
                       << state_mapping.first << "\n";
                  });
    return os.str();
  }

  template <class _Automaton>
  static std::string current_state_info(const _Automaton &automaton) {
    std::ostringstream os;
    // os << "Current State,State Hash\n" +
    //           std::string(typeid(automaton._get_current_state()).name()) +
    //           "," + std::to_string(current_state_hash(automaton)) + "\n";
    return os.str();
  }
};

template <typename _InputType, typename _OutputType> class State {
public:
  using base_t = State<_InputType, _OutputType>;
  using input_t = _InputType;
  using output_t = _OutputType;

  std::pair<output_t, size_t> process(input_t input) {
    return {_next_state(input), _next_output(input)};
  }

protected:
  virtual output_t _next_output(input_t input) = 0;
  virtual size_t _next_state(input_t input) = 0;
};

template <typename _InputType, typename _OutputType, class _State>
struct is_state
    : public std::is_base_of<State<_InputType, _OutputType>, _State> {};

template <typename _InputType, typename _OutputType, class _State>
inline constexpr bool is_state_v =
    is_state<_InputType, _OutputType, _State>::value;

template <typename _InputType, typename _OutputType, class _S0, class... _Sn>
inline constexpr bool all_states_v =
    ((is_state_v<_InputType, _OutputType, _S0> &&
      is_state_v<_InputType, _OutputType, _Sn>) &&
     ...);

template <class _S0, class... _Sn>
using state_tuple_t = std::tuple<_S0, _Sn...>;

// A state type that's a member of _S0..._Sn: x: x is
template <class _S0, class... _Sn>
using state_variant_t = std::variant<_S0, _Sn...>;

template <class _S0, class... _Sn>
using state_map_t = std::unordered_map<size_t, state_variant_t<_S0, _Sn...>>;

template <class _S0, class... _Sn>
class StateMap : protected state_map_t<_S0, _Sn...> {
public:
  using state_set_t = state_tuple_t<_S0, _Sn...>;
  using variant_t = state_variant_t<_S0, _Sn...>;
  using map_t = state_map_t<_S0, _Sn...>;
  using key_t = typename state_map_t<_S0, _Sn...>::key_type;
  using value_t = typename state_map_t<_S0, _Sn...>::mapped_type;
  inline constexpr static size_t state_count = 1 + sizeof...(_Sn);

public:
  StateMap()
      : map_t({{type_hash_v<_S0>, variant_t((_S0()))},
               {type_hash_v<_Sn>, variant_t(_Sn())}...}) {}

  StateMap(_S0 s0, _Sn... sn)
      : map_t({{type_hash_v<_S0>, variant_t(s0)},
               {type_hash_v<_Sn>, variant_t(sn)}...}) {}

  template <typename Arg, typename... Args>
  StateMap(Arg arg, Args... args)
      : map_t({{type_hash_v<_S0>, variant_t(_S0(arg))},
               {type_hash_v<_Sn>, variant_t(_Sn(args))}...}) {
    static_assert(sizeof...(args) == state_count - 1);
  }

  // ((is_state_v<_InputType, _OutputType, _S0> &&
  //   is_state_v<_InputType, _OutputType, _Sn>) &&
  //  ...);
  template <class _State> value_t &get() {
    // static_assert(((std::is_same_v<_State, _S0> ||
    //               std::is_same_v<_State, _Sn>)) || ...));
    return this->at(type_hash_v<_State>);
  }

  value_t &get(size_t state_hash) { return this->at(state_hash); }
};

template <typename _InputType, typename _OutputType, class _S0, class... _Sn>
class Automaton {
  using initial_state_t = _S0;
  using base_state_t = typename initial_state_t::base_t;
  using state_input_t = typename base_state_t::input_t;
  using state_output_t = typename base_state_t::output_t;
  using state_set_t = StateMap<initial_state_t, _Sn...>;

  static_assert(
      all_states_v<state_input_t, state_output_t, initial_state_t, _Sn...>);

protected:
  state_set_t _states;
  typename state_set_t::value_t _current_state;

public:
  Automaton()
      : _states(), _current_state(_states.template get<initial_state_t>()) {}

  Automaton &reset() {
    *this = Automaton();
    return *this;
  }

  state_output_t transduce(state_input_t input) {
    auto [output, next_state] = _current_state.process(input);
    _current_state = _states.get(next_state);
    return output;
  }
};

// template <typename _InputType, typename _OutputType,
//           template <typename, typename> class _S0,
//           template <typename, typename> class... _Sn>
// class SequentialAutomaton
//     : public Automaton<_InputType, _OutputType, _S0, _Sn...> {};

namespace fsm {
template <class _Obj, typename _IsFinal> class State {
  static_assert(std::is_convertible_v<_IsFinal, bool>,
                "`_IsFinal` must be convertible to a boolean type.");

public:
  static constexpr bool is_final = _IsFinal::value;
  using obj_t = _Obj;
  using weak_ptr_t = std::weak_ptr<_Obj>;
  using shared_ptr_t = std::shared_ptr<_Obj>;
  State(const shared_ptr_t &) {}

  constexpr size_t get_hash() const {
    return state_type_hash_v<decltype(*this)>;
  }

  /*
  Default implementation of a state's `process` function, returning the
  hash value of itself and a nullptr for output.

  Derived states should create specializations of this function
  to handle specific classes of input.

  The return value shall be the hash value of a pointer-to-state type.
  Below is a valid example of such type:

    `typeid(MyStateType *).hash_code();`

  Note:
  The templated variable `state_type_hash_v` is available for convenience.
  */
  template <typename... _Args>
  [[nodiscard]] std::pair<size_t, std::nullptr_t>
  process([[maybe_unused]] _Args...) {
    return {state_type_hash_v<decltype(*this)>, nullptr};
  }
};

// Aliases
// using state_t = State<std::false_type>;
// using final_state_t = State<std::true_type>;
}; // namespace fsm