#include "../include/FiniteStateMachine.hpp"
#include "fsm/even_zeros.hpp"
#include "fsm/incrementer.hpp"
#include "fsm/turnstile.hpp"
#include <cassert>

void test_incrementer_fsm() {
  auto fsm = incrementer_fsm_t();
  static_assert(fsm.has_final_state);

  // Default values
  assert(!fsm.has_started());
  assert(!fsm.is_terminated());
  assert(!fsm.in_final_state());
  assert(fsm == 0);

  // Run until final state
  while (!fsm.in_final_state()) {
    fsm.process();
  }

  assert(fsm.has_started());    // Implicit start via process function
  assert(fsm.in_final_state()); // FinalState
  assert(fsm == 100);           // CheckState -> FinalState is reached at 100

  fsm.terminate();
  assert(fsm.is_terminated());

  // Resetting the FSM should have default values.
  fsm.reset();
  assert(!fsm.has_started());
  assert(!fsm.is_terminated());
  assert(fsm == 0);

  // Terminating an unstarted FSM won't do anything.
  fsm.terminate();
  assert(!fsm.is_terminated());
}

void test_turnstile_fsm() {
  auto fsm = TurnstileExample::turnstile_fsm_t();
  static_assert(!fsm.has_final_state);
  assert(!fsm.in_final_state());
  assert(!fsm.has_started());
  assert(!fsm.is_terminated());
  assert(fsm->is_locked() == true);
  fsm.process(TurnstileExample::Input::coin);
  assert(!fsm->is_locked());
  fsm.process(TurnstileExample::Input::coin);
  assert(!fsm->is_locked());
  fsm.process(TurnstileExample::Input::push);
  assert(fsm->is_locked());
  fsm.process(TurnstileExample::Input::push);
  assert(fsm->is_locked());
  assert(!fsm.in_final_state());
}

void test_even_zeros() {
  auto fsm = EvenZerosExample::even_zeros_acceptor_t();
  assert(fsm.is_accepted());
  assert(!fsm.is_accepted(0));
  assert(fsm.is_accepted(0));
  // assert(fsm.is_accepted(0, 0));
}

int main() {
  test_incrementer_fsm();
  test_turnstile_fsm();
}