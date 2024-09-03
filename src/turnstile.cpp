#include "../include/state_machine.hpp"
#include <iostream>
#include <unordered_map>

enum class TurnstileStateValues { Locked, Unlocked };
enum class TurnstileInputValues { Coin, Push };
class TurnstileState : public State<TurnstileInputValues> {};

class LockedState : public TurnstileState {
  void on_enter(TurnstileInputValues input) override {}
  void on_exit(TurnstileInputValues input) override {
    switch (input) {
    case TurnstileInputValues::Coin:
      std::cout
          << "Turnstile received coin while locked. Unlocking turnstile.\n";
      break;
    default:
      std::cout << "Turnstile received input that's not a coin while locked. "
                   "Turnstile still locked.\n";
      break;
    }
  }
};

class UnlockedState : public TurnstileState {
  void on_enter(TurnstileInputValues input) override {}
  void on_exit(TurnstileInputValues input) override {
    switch (input) {
    case TurnstileInputValues::Push:
      std::cout
          << "Turnstile received push while unlocked. Locking turnstile.\n";
      break;
    default:
      std::cout << "Turnstile received coin while unlocked. Turnstile still "
                   "unlocked.\n";
      break;
    }
  }
};

class Turnstile : public FSM<TurnstileState, TurnstileInputValues> {
private:
  LockedState locked_state;
  UnlockedState unlocked_state;

public:
  Turnstile() {
    std::unordered_map<TurnstileInputValues, TurnstileState *>
        locked_transitions{{TurnstileInputValues::Coin, &unlocked_state},
                           {TurnstileInputValues::Push, &locked_state}};

    std::unordered_map<TurnstileInputValues, TurnstileState *>
        unlocked_transitions{{TurnstileInputValues::Coin, &unlocked_state},
                             {TurnstileInputValues::Push, &locked_state}};

    std::unordered_map<
        TurnstileState *,
        std::unordered_map<TurnstileInputValues, TurnstileState *>>
        transitions = {{&locked_state, locked_transitions},
                       {&unlocked_state, unlocked_transitions}};

    reset(&locked_state, transitions);
  }

  void push() { input(TurnstileInputValues::Push); }
  void insert_coin() { input(TurnstileInputValues::Coin); }
};

int main() {
  auto turnstile = Turnstile();
  turnstile.push();
  turnstile.insert_coin();
  turnstile.insert_coin();
  turnstile.push();
  turnstile.push();
}