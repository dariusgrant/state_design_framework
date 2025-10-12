#pragma once

#include "AbstractState.hpp"
#include "ProcessStrategy.hpp"
#include "Subscription.hpp"
#include "utility/Concurrency.hpp"
#include "utility/Environment.hpp"
#include "utility/StateType.hpp"
#include <algorithm>
#include <atomic>

#include <cstddef>
#include <execution>
#include <functional>
#include <future>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <variant>

namespace fsm {
class BaseFiniteStateMachine {
protected:
  std::atomic_bool _started;
  std::atomic_bool _terminated;

public:
  BaseFiniteStateMachine() : _started(false), _terminated(false) {}

  bool has_started() const { return _started.load(); }
  bool is_terminated() const { return _terminated.load(); }

  BaseFiniteStateMachine &start() {
    if (!has_started()) {
      _started.store(true);
    }
    return *this;
  }

  BaseFiniteStateMachine &terminate() {
    if (_started.load()) {
      _terminated.store(true);
    }
    return *this;
  }
};

template <class _Obj, class _S0, class... _Sn>
class FiniteStateMachine : public BaseFiniteStateMachine {
  // Reject parameter type that isn't a state.
  static_assert((std::is_base_of_v<NonTerminalState<_Obj>, _S0> ||
                 std::is_base_of_v<TerminalState<_Obj>, _S0>) ||
                    ((std::is_base_of_v<NonTerminalState<_Obj>, _Sn> ||
                      std::is_base_of_v<TerminalState<_Obj>, _Sn>) ||
                     ...),
                "The state does not derived from class `AbstractState`");

public:
  using shared_ptr_t = std::shared_ptr<_Obj>;
  using shared_future_t = std::shared_future<_Obj>;
  using state_tuple_t = StateTuple<_S0, _Sn...>;
  using state_variant_t = StateVariant<_S0 *, _Sn *...>;
  using future_subscription_t = FutureSubscription<_Obj>;

  static constexpr bool has_terminal_state =
      (std::is_base_of_v<fsm::TerminalState<_Obj>, _S0> ||
       (std::is_base_of_v<fsm::TerminalState<_Obj>, _Sn> || ...));

protected:
  shared_ptr_t _object;
  state_tuple_t _states;
  state_variant_t _current_state;
  /*
  The FSM can notify subscribers in 2 different ways:
    1. Direct
    2. Queued

  Direct notification invokes a subscriber's callback function
  as the subscription mechanism. This type of notification is
  constrained to single-threaded processing as the invocation
  is ran in the same thread as the notifying FSM.

  Queued notification uses a subscriber's queue by pushing the
  notification into it, free of any additional invocations.
  This type of notification is intended for asynchronous
  processing.
  */
  future_subscription_t _shared_output;
  AtomicQueue<shared_future_t> _queue;

public:
  template <typename... _ObjArgs>
  FiniteStateMachine(_ObjArgs... args)
      : _object(std::make_shared<_Obj>(args...)),
        _states(std::make_tuple(_S0(_object), _Sn(_object)...)),
        _current_state(&std::get<0>(_states)) {}

  const _Obj &get() const { return *_object; }

  template <class _Fsm> FiniteStateMachine &hook(std::string name, _Fsm &fsm) {
    _shared_output.add(name, fsm);
    return *this;
  }

  template <typename _Strat = DefaultProcessStrategy, typename... _Args>
  FiniteStateMachine &process(_Args... args) {
    static_assert(std::is_base_of_v<AbstractProcessStrategy, _Strat>,
                  "`_Strat` must derive from `AbstractProcessStrategy`");
    if (is_terminated()) {
      if constexpr (THROW_ON_PROCESS_AFTER_TERMINATION) {
        throw std::runtime_error("FSM already terminated.");
      }
      return *this;
    }

    if (has_started()) {
      _Strat::process(std::function<void(_Args...)>(
                          [&](_Args... args) { _enter_state(args...); }),
                      std::function<void(_Args...)>(
                          [&](_Args... args) { _exit_state(args...); }),
                      std::function<void(_Args...)>(
                          [&](_Args... args) { _transition_state(args...); }),
                      args...);
    } else {
      start();
      _enter_state(args...);
    }
    return *this;
  }

private:
  template <typename... _Args> void _enter_state(_Args... args) {
    shared_future_t future_obj;
    std::visit(
        [&](auto &s) {
          future_obj = std::async(std::launch::deferred, [&]() -> _Obj {
            s->enter(args...);
            if (s->is_terminal) {
              _terminated = true;
            }
            return *_object;
          });
        },
        _current_state);

    *_object = future_obj.get();
    _notify_subscribers(future_obj);
  }

  template <typename... _Args> void _exit_state(_Args... args) {
    std::visit([&](auto &s) { s->exit(args...); }, _current_state);
  }

  template <typename... _Args> void _transition_state(_Args... args) {
    std::visit(
        [&](auto &s) {
          auto next_state_identity = s->transition(args...);
          if (std::is_same_v<typename decltype(next_state_identity)::type,
                             std::nullptr_t>) {
            _terminated.store(true);
            return;
          }
          _current_state =
              &std::get<typename decltype(next_state_identity)::type>(_states);
        },
        _current_state);
  }

  void _notify_subscribers(std::shared_future<_Obj> future_obj) {
    std::for_each(std::execution::par_unseq, _shared_output.begin(),
                  _shared_output.end(),
                  [&](auto &sub_cb) { sub_cb.second(future_obj); });
  }
};
} // namespace fsm
