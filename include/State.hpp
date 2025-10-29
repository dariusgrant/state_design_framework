#pragma once
#include "utility/StateType.hpp"
#include <cstddef>
#include <memory>
#include <type_traits>

namespace fsm {
template <class _Obj, typename _IsFinal> class State {
  static_assert(std::is_convertible_v<_IsFinal, bool>,
                "`_IsFinal` must be convertible to a boolean type.");

public:
  using obj_t = _Obj;
  using weak_ptr_t = std::weak_ptr<_Obj>;
  using shared_ptr_t = std::shared_ptr<_Obj>;

protected:
  weak_ptr_t _obj;

public:
  static constexpr bool is_final = _IsFinal::value;

  State(const shared_ptr_t &obj) : _obj(obj) {}

  obj_t &get_object() {
    if (_obj.expired()) {
      throw;
    }
    return *_obj.lock();
  }

  /*
  Default implementation of a state's `process` function, returning the
  hash value of itself.

  Derived states should create specializations of this function
  to handle specific classes of input.

  The return value shall be the hash value of a pointer-to-state type.
  Below is a valid example of such type:

    `typeid(MyStateType *).hash_code();`

  Note:
  The templated variable `state_type_hash_v` is available for convenience.
  */
  template <typename... _Args>
  [[nodiscard]] size_t process([[maybe_unused]] _Args...) {
    return state_type_hash_v<decltype(*this)>;
  }
};

// Aliases
template <class _Obj> using state_t = State<_Obj, std::false_type>;
template <class _Obj> using final_state_t = State<_Obj, std::true_type>;
}; // namespace fsm