#pragma once
#include "utility/StateType.hpp"
#include <cstddef>
#include <memory>
#include <type_traits>

namespace fsm {
template <class _Obj, typename _IsTerminal> class State {
  static_assert(std::is_convertible_v<_IsTerminal, bool>,
                "`_IsTerminal` must be convertible to a boolean type.");

public:
  using obj_t = _Obj;
  using weak_ptr_t = std::weak_ptr<_Obj>;
  using shared_ptr_t = std::shared_ptr<_Obj>;

protected:
  weak_ptr_t _obj;

public:
  static constexpr bool is_terminal = _IsTerminal::value;

  State(const shared_ptr_t &obj) : _obj(obj) {}

  obj_t &get_object() {
    if (_obj.expired()) {
      throw;
    }
    return *_obj.lock();
  }

  void enter() {}
  template <typename... _Args> void enter(_Args...) {}

  void exit() {}
  template <typename... _Args> void exit(_Args...) {}

  [[nodiscard]] size_t transition() {
    return state_type_hash_v<decltype(*this)>;
  }

  template <class _T, typename... _Args>
  [[nodiscard]] size_t transition(_Args...) {
    return state_type_hash_v<decltype(*this)>;
  }
};

// Aliases
template <class _Obj> using non_terminal_state_t = State<_Obj, std::false_type>;
template <class _Obj> using terminal_state_t = State<_Obj, std::true_type>;
}; // namespace fsm