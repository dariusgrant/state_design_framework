#include "../include/FiniteStateMachine.hpp"
#include "fsm/incrementer.hpp"
#include <cassert>

void test_incrementer_fsm() {
  auto fsm = incrementer_fsm_t();
  // FinalState is the terminal state
  static_assert(fsm.has_terminal_state);

  // Default values
  assert(!fsm.has_started());
  assert(!fsm.is_terminated());
  assert(fsm == 0);

  // Run until termination
  while (!fsm.is_terminated()) {
    fsm.process();
  }

  assert(fsm.has_started());   // Implicit start via process function
  assert(fsm.is_terminated()); // Termination via FinalState entrance
  assert(fsm == 100);          // CheckState -> FinalState is reached at 100

  // Resetting the FSM should have default values.
  fsm.reset();
  assert(!fsm.has_started());
  assert(!fsm.is_terminated());
  assert(fsm == 0);

  // Terminating an unstarted FSM won't do anything.
  fsm.terminate();
  assert(!fsm.is_terminated());
}

int main() { test_incrementer_fsm(); }