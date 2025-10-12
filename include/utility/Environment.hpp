#pragma once

namespace fsm {
#ifndef NDEBUG
constexpr bool DEBUG_MODE = true;
#else
constexpr bool DEBUG_MODE = false;
#endif

#ifndef FSM_THROW_ON_PROCESS_AFTER_TERMINATION
constexpr bool THROW_ON_PROCESS_AFTER_TERMINATION = false;
#else
constexpr bool THROW_ON_PROCESS_AFTER_TERMINATION = false;
#endif

// Fall through to `AbstractState::[enter | exit]` functions if no definition
// exist in derived state..
#ifndef FSM_ABSTRACT_STATE_FALLTHROUGH
constexpr bool ABSTRACT_STATE_FALLTHROUGH = false;
#else
constexpr bool ABSTRACT_STATE_FALLTHROUGH = true;
#endif
}; // namespace fsm
