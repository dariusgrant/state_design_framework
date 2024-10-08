#include "../include/state_machine.hpp"
#include <iostream>
#include <unordered_map>

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
  void output(std::string message) { std::cout << message; }
};

/*
Step 2:
Create the input set that can be accepted by the FSM.
*/
enum class TurnstileInputEnum { None, Coin, Push };

/*
Step 3:
Create the set of states the FSM will use to transition between.
*/
using TurnstileState = State<Turnstile, TurnstileInputEnum>;
class LockedState : public TurnstileState {
public:
  LockedState(Turnstile *turnstile) : State(turnstile) {}
  void on_enter(TurnstileInputEnum input) override {
    object->lock();
    object->output("Turnstile in locked state.\n");
  }
  void on_exit(TurnstileInputEnum input) override {
    switch (input) {
    case TurnstileInputEnum::Coin:
      object->output("Turnstile received coin while locked.\n");
      break;
    default:
      object->output(
          "Turnstile received input that's not a coin while locked.\n");
      break;
    }
  }
};

class UnlockedState : public TurnstileState {
public:
  UnlockedState(Turnstile *turnstile) : State(turnstile) {}

  void on_enter(TurnstileInputEnum input) override {
    object->unlock();
    object->output("Turnstile in unlocked state.\n");
  }
  void on_exit(TurnstileInputEnum input) override {
    switch (input) {
    case TurnstileInputEnum::Push:
      object->output("Turnstile received push while unlocked.\n");
      break;
    default:
      object->output("Turnstile received coin while unlocked.\n");
      break;
    }
  }
};

/*
Step 4:
Create the FSM.
*/
class TurnstileFSM : public Turnstile, FSM<TurnstileState, TurnstileInputEnum> {
private:
  LockedState locked_state;
  UnlockedState unlocked_state;

public:
  TurnstileFSM() : locked_state(this), unlocked_state(this) {
    std::unordered_map<TurnstileInputEnum, TurnstileState *> locked_transitions{
        {TurnstileInputEnum::Coin, &unlocked_state},
        {TurnstileInputEnum::Push, &locked_state}};

    std::unordered_map<TurnstileInputEnum, TurnstileState *>
        unlocked_transitions{{TurnstileInputEnum::Coin, &unlocked_state},
                             {TurnstileInputEnum::Push, &locked_state}};

    std::unordered_map<TurnstileState *,
                       std::unordered_map<TurnstileInputEnum, TurnstileState *>>
        transitions = {{&locked_state, locked_transitions},
                       {&unlocked_state, unlocked_transitions}};

    reset(&locked_state, transitions);
    start(TurnstileInputEnum::None);
  }

  void push() { input(TurnstileInputEnum::Push); }
  void insert_coin() { input(TurnstileInputEnum::Coin); }
};

/*
Step 5:

*/
int main() {
  auto turnstile = TurnstileFSM();
  turnstile.push();
  turnstile.insert_coin();
  turnstile.insert_coin();
  turnstile.push();
  turnstile.push();
}