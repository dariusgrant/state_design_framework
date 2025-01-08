#include "../include/finite_state_machine.hpp"
#include <any>
#include <iostream>

enum class TurnstileInputEnum { Coin, Push };
/*
Step 1:
Create the object that will be manipulated by the FSM.
*/
class Turnstile {
private:
  bool is_locked;

public:
  Turnstile() {}

  void lock() { is_locked = true; }
  void unlock() { is_locked = false; }
};

using TurnstileState = state<Turnstile>;
using TurnstileFSM = finite_state_machine<Turnstile>;

class LockedState : public TurnstileState {
public:
  LockedState()
      : state("Locked",
              create_on_access_func<LockedState>(this, access_type::enter),
              create_on_access_func<LockedState>(this, access_type::exit),
              create_state_proc_func<LockedState>(this)) {}

  void on_enter(Turnstile &turnstile) {
    turnstile.lock();
    std::cout << "Turnstile in locked state.\n";
  }

  void on_exit(Turnstile &turnstile) {
    std::cout << "Turnstile exiting locked state.\n";
  }

  void process(TurnstileFSM &fsm, std::any input) {
    try {
      auto turnstile_input = std::any_cast<TurnstileInputEnum>(input);
      if (turnstile_input == TurnstileInputEnum::Coin) {
        fsm.set_state("Unlocked", "Request to unlock turnstile granted.");
      }
    } catch (std::bad_any_cast &e) {
      std::cout << "Bad input for locked state";
    }
  }
};

class UnlockedState : public TurnstileState {
public:
  UnlockedState()
      : state("Unlocked",
              create_on_access_func<UnlockedState>(this, access_type::enter),
              create_on_access_func<UnlockedState>(this, access_type::exit),
              create_state_proc_func<UnlockedState>(this)) {}

  void on_enter(Turnstile &turnstile) {
    turnstile.unlock();
    std::cout << "Turnstile in unlocked state.\n";
  }

  void on_exit(Turnstile &turnstile) {
    std::cout << "Turnstile exiting unlocked state.\n";
  }

  void process(TurnstileFSM &fsm, std::any input) {
    try {
      auto turnstile_input = std::any_cast<TurnstileInputEnum>(input);
      if (turnstile_input == TurnstileInputEnum::Push) {
        fsm.set_state("Locked", "Request to unlock turnstile granted.");
      } else {
      }
    } catch (std::bad_any_cast &e) {
      std::cout << "Bad input for unlocked state";
    }
  }
};

class TurnstileMachine : public TurnstileFSM {
private:
public:
  TurnstileMachine(Turnstile turnstile = Turnstile())
      : TurnstileFSM(turnstile, {LockedState(), UnlockedState()}, "Locked") {}

  void push() {
    process(std::make_any<TurnstileInputEnum>(TurnstileInputEnum::Push));
  }

  void insert_coin() {
    process(std::make_any<TurnstileInputEnum>(TurnstileInputEnum::Coin));
  }
};

/*
Step 5:
*/
struct Person {
  void push(TurnstileMachine &t) { t.push(); }
  void insert_coin(TurnstileMachine &t) { t.insert_coin(); }
};

int main() {
  auto turnstile = TurnstileMachine();
  Person p;
  p.push(turnstile);
  p.insert_coin(turnstile);
  p.push(turnstile);
}