#pragma once
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <variant>

namespace fsm {
using UnhandledParameters = std::false_type::type;

template <typename IsTerminal> class AbstractState {
protected:
public:
  static constexpr bool is_terminal = IsTerminal::value;

  template <class ObjectType, typename T>
  void enter(ObjectType &object, T input) {
    static_assert(UnhandledParameters::value,
                  "`enter` not defined for parameter list.");
  }

  template <class ObjectType, typename T>
  void exit(const ObjectType &object, T input) {
    if constexpr (is_terminal) {
      return;
    }
    static_assert(UnhandledParameters::value,
                  "`exit` not defined for parameter list.");
  }

  template <class ObjectType, typename T>
  std::type_index transition(const ObjectType &object, T input) {
    static_assert(UnhandledParameters::value,
                  "`transition` not defined for parameter list.");
    return std::type_index(typeid(this));
  }

  template <typename... Ts> void configure(Ts... args) {
    static_assert(UnhandledParameters::value,
                  "`configure` not defined for parameter list.");
  }
};

using NonTerminalState = AbstractState<std::false_type>;
using TerminalState = AbstractState<std::true_type>;
}; // namespace fsm