#pragma once

#include <functional>

namespace fsm {
struct AbstractProcessStrategy {
  template <typename... _Args> using callback_t = std::function<void(_Args...)>;

  template <typename... _Args>
  constexpr static void
  process(callback_t<_Args...> enter, callback_t<_Args...> exit,
          callback_t<_Args...> transition, _Args... args) {
    static_assert(
        std::false_type::value,
        "`AbstractProcessStrategy` cannot be used as a process strategy.");
  };
};

struct DefaultProcessStrategy : public AbstractProcessStrategy {
  template <typename... _Args>
  constexpr static void
  process(callback_t<_Args...> enter, callback_t<_Args...> exit,
          callback_t<_Args...> transition, _Args... args) {
    exit(args...);
    transition(args...);
    enter(args...);
  };
};
} // namespace fsm
