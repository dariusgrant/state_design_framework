#pragma once

#include "../../include/FiniteStateMachine.hpp"
#include <cassert>
namespace TurnstileExample {
enum class Input { coin, push };

class Turnstile {
private:
  bool _is_locked;

public:
  Turnstile() : _is_locked(true) {}
  void lock() { _is_locked = true; }
  void unlock() { _is_locked = false; }
  bool is_locked() const { return _is_locked; }
};

using non_terminal_turnstile_state_t = fsm::non_terminal_state_t<Turnstile>;

class LockedState;
class UnlockedState;
using turnstile_fsm_t =
    fsm::FiniteStateMachine<Turnstile, LockedState, UnlockedState>;

class LockedState : public non_terminal_turnstile_state_t {
public:
  LockedState(const shared_ptr_t &s) : non_terminal_turnstile_state_t(s) {}
  size_t process(Input i) {
    auto &turnstile = get_object();
    if (i == Input::coin) {
      turnstile.unlock();
      return fsm::state_type_hash_v<UnlockedState>;
    } else {
      return fsm::state_type_hash_v<LockedState>;
    }
  }
};

class UnlockedState : public non_terminal_turnstile_state_t {
public:
  UnlockedState(const shared_ptr_t &s) : non_terminal_turnstile_state_t(s) {}
  size_t process(Input i) {
    auto &turnstile = get_object();
    if (i == Input::push) {
      turnstile.lock();
      return fsm::state_type_hash_v<LockedState>;
    } else {
      return fsm::state_type_hash_v<UnlockedState>;
    }
  }
};
}; // namespace TurnstileExample