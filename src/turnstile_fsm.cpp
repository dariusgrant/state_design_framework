#include "../include/finite_state_machine.hpp"
#include <functional>
#include <iostream>

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
Step 3:
Create the set of states the FSM will use to transition between.
*/
using TurnstileState = state<Turnstile>;
using TurnstileFSM = finite_state_machine<Turnstile>;

class LockedState : public TurnstileState {
public:
  void process(TurnstileFSM &fsm, std::any input) {
    auto turnstile = fsm.get_object();
    turnstile->lock();
    turnstile->output("Turnstile in locked state.\n");
  }

  LockedState() : state("Locked", create_state_proc_func<LockedState>(this)) {}
};

// class UnlockedState : public TurnstileState {
// public:
//   UnlockedState() : state("Unlocked",
//   _create_process_func(&UnlockedState::process, this)) {}

//   void on_enter(TurnstileInputEnum input) override {
//     object->unlock();
//     object->output("Turnstile in unlocked state.\n");
//   }
//   void on_exit(TurnstileInputEnum input) override {
//     switch (input) {
//     case TurnstileInputEnum::Push:
//       object->output("Turnstile received push while unlocked.\n");
//       break;
//     default:
//       object->output("Turnstile received coin while unlocked.\n");
//       break;
//     }
//   }
// };

class TurnstileMachine : public TurnstileFSM {
public:
  TurnstileMachine(Turnstile turnstile = Turnstile())
      : TurnstileFSM(turnstile, {LockedState()}, "Locked"){};
};

/*
Step 4:
Create the FSM.
*/
// class TurnstileFSM : public Turnstile, FSM<TurnstileState,
// TurnstileInputEnum> { private:
//   LockedState locked_state;
//   UnlockedState unlocked_state;

// public:
//   TurnstileFSM() : locked_state(this), unlocked_state(this) {
//     std::unordered_map<TurnstileInputEnum, TurnstileState *>
//     locked_transitions{
//         {TurnstileInputEnum::Coin, &unlocked_state},
//         {TurnstileInputEnum::Push, &locked_state}};

//     std::unordered_map<TurnstileInputEnum, TurnstileState *>
//         unlocked_transitions{{TurnstileInputEnum::Coin, &unlocked_state},
//                              {TurnstileInputEnum::Push, &locked_state}};

//     std::unordered_map<TurnstileState *,
//                        std::unordered_map<TurnstileInputEnum, TurnstileState
//                        *>>
//         transitions = {{&locked_state, locked_transitions},
//                        {&unlocked_state, unlocked_transitions}};

//     reset(&locked_state, transitions);
//     start(TurnstileInputEnum::None);
//   }

//   void push() { input(TurnstileInputEnum::Push); }
//   void insert_coin() { input(TurnstileInputEnum::Coin); }
// };

/*
Step 5:

*/
int main() {
  auto turnstile = TurnstileMachine();
  turnstile.process(nullptr);
}