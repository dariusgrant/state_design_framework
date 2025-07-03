#include <stdexcept>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <variant>

namespace fsm {
// Get the type index of pointer to State type.
// This allows classes to forward declare for cyclic dependencies.
template <class State>
auto GetStateTypeIndex =
    []() -> std::type_index { return std::type_index(typeid(State *)); };

template <class... StateTypes>
using StateMap =
    std::unordered_map<std::type_index, std::variant<StateTypes...>>;

class InvalidStateArgument : public std::invalid_argument {
public:
  InvalidStateArgument(std::string s)
      : std::invalid_argument("Failed to handle arguments in state " + s) {}
};

class StartedFSM : public std::logic_error {
public:
  StartedFSM() : std::logic_error("FSM has already been started") {}
};

class UnstartedFSM : public std::logic_error {
public:
  UnstartedFSM() : std::logic_error("FSM hasn't been started") {}
};

class AbstractState {
public:
  template <class ObjectType, typename T>
  void enter(ObjectType &object, T input) {
    throw InvalidStateArgument("enter");
  }

  template <class ObjectType, typename T>
  void exit(const ObjectType &object, T input) {
    throw InvalidStateArgument("exit");
  }

  template <class ObjectType, typename T, class... StateTypes>
  std::variant<StateTypes...> &
  transition(const ObjectType &object, T input,
             const StateMap<StateTypes...> &states) {
    throw InvalidStateArgument("transition");
  }

  template <typename... Ts> void configure(Ts... args) {
    throw InvalidStateArgument("configuration");
  }
};
}; // namespace fsm