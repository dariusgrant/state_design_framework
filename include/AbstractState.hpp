#pragma once
#include "utility/Environment.hpp"
#include "utility/StateType.hpp"
#include <memory>
#include <type_traits>
#include <typeindex>
#include <utility>

namespace fsm {
using UnhandledParameters = std::false_type::type;

template <class ObjType, typename IsTerminal> class AbstractState {
public:
  using obj_t = ObjType;
  using weak_ptr_t = std::weak_ptr<ObjType>;
  using shared_ptr_t = std::shared_ptr<ObjType>;

protected:
  weak_ptr_t _obj;

public:
  static constexpr bool is_terminal = IsTerminal::value;

  AbstractState(){};
  AbstractState(const shared_ptr_t &obj) : _obj(obj) {}
  AbstractState(AbstractState &s) : _obj(s._obj) {}
  AbstractState(const AbstractState &s) : _obj(s._obj) {}
  ~AbstractState() {}
  template <typename... T> void enter(T... inputs) {
    // if (!_obj.expired()) {
    //   _enter_impl(*_obj.lock(), std::forward<T>(inputs)...);
    // }
  }

  template <typename... T> void exit(T... inputs) {
    // if (!_obj.expired()) {
    //   _exit_impl(std::as_const(*_obj.lock()), std::forward<T>(inputs)...);
    // }
  }

  template <typename... T>
  [[nodiscard]] std::type_index transition(T... inputs) {
    // if (!_obj.expired()) {
    //   return _transition_impl(std::as_const(*_obj.lock()),
    //                           std::forward<T>(inputs)...);
    // }
    return std::type_index(typeid(nullptr));
  }

  template <typename... Ts> void configure(Ts... args) {
    static_assert(UnhandledParameters::value,
                  "`configure` not defined for parameter list.");
  }

  // protected:
  //   template <typename... T> void _enter_impl(T... inputs) {
  //     if constexpr (!ABSTRACT_STATE_FALLTHROUGH) {
  //       static_assert(std::false_type::value,
  //                     "`_enter_impl` not defined for parameter list.");
  //     }
  //   }

  //   template <typename... T> void _exit_impl(T... inputs) {
  //     if constexpr (is_terminal) {
  //       return;
  //     }

  //     if constexpr (!ABSTRACT_STATE_FALLTHROUGH) {
  //       static_assert(std::false_type::value,
  //                     "`_exit_impl` not defined for parameter list.");
  //     }
  //   }

  //   template <typename... T> std::type_index _transition_impl(T... inputs) {
  //     if constexpr (!ABSTRACT_STATE_FALLTHROUGH) {
  //       static_assert(std::false_type::value,
  //                     "`_transition_impl` not defined for parameter list.");
  //     }
  //   }
};

template <class ObjType>
using NonTerminalState = AbstractState<ObjType, std::false_type>;

template <class ObjType>
using TerminalState = AbstractState<ObjType, std::true_type>;

template <class ObjType> using MonoNonTerminalState = NonTerminalState<ObjType>;
}; // namespace fsm