#pragma once
#include "utility/Environment.hpp"
#include <cstddef>
#include <memory>
#include <type_traits>

#define VARIADIC_TEMPLATED_FUNCTION_NO_OP(return_type, name)                   \
  template <typename... _Args>                                                 \
  return_type name([[maybe_unused]] _Args... args) {}

#define STATE_ENTER_FUNCTION_NO_OP                                             \
  VARIADIC_TEMPLATED_FUNCTION_NO_OP(void, enter)

#define STATE_EXIT_FUNCTION_NO_OP VARIADIC_TEMPLATED_FUNCTION_NO_OP(void, exit)

namespace fsm {
template <class _T> struct StateIdentity {
  using type = _T;
};

template <class _Obj, typename _IsTerminal> class AbstractState {
public:
  using obj_t = _Obj;
  using weak_ptr_t = std::weak_ptr<_Obj>;
  using shared_ptr_t = std::shared_ptr<_Obj>;

protected:
  weak_ptr_t _obj;

public:
  static constexpr bool is_terminal = _IsTerminal::value;

  AbstractState(const shared_ptr_t &obj) : _obj(obj) {}

  obj_t &get_object() {
    if (_obj.expired()) {
      throw;
    }
    return *_obj.lock();
  }

  template <typename... _Args> void enter([[maybe_unused]] _Args... args) {
    static_assert(ABSTRACT_STATE_FALLTHROUGH,
                  "`enter` not defined for parameter list.");
  }

  template <typename... T> void exit([[maybe_unused]] T... args) {
    if constexpr (is_terminal) {
      return;
    }
    static_assert(ABSTRACT_STATE_FALLTHROUGH,
                  "`exit` not defined for parameter list.");
  }

  template <class _T, typename... _Args>
  [[nodiscard]] StateIdentity<_T> transition([[maybe_unused]] _Args... args) {
    return StateIdentity<std::nullptr_t>();
  }
};

template <class _Obj>
using NonTerminalState = AbstractState<_Obj, std::false_type>;

template <class _Obj> using TerminalState = AbstractState<_Obj, std::true_type>;
}; // namespace fsm