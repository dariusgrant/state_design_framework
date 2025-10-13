#pragma once

#include <atomic>
#include <iostream>
#include <list>
#include <memory>
#include <queue>
#include <thread>

namespace fsm {
class ThreadOwnership {
private:
  static inline std::thread::id _default_thread_id = std::thread::id();
};

template <class _Obj>
class AtomicQueue : public std::queue<_Obj, std::list<_Obj>> {
public:
  using container_t = std::queue<_Obj, std::list<_Obj>>;

private:
  static inline std::thread::id _default_thread_id = std::thread::id();
  std::atomic<std::thread::id> _current_thread;

public:
  AtomicQueue() : container_t(), _current_thread(std::thread::id()) {}

  void push(_Obj obj) {
    _wait_for_ownership();
    static_cast<container_t *>(this)->push(obj);
    _current_thread.store(_default_thread_id);
  }

  _Obj pop() {
    _wait_for_ownership();
    auto res = static_cast<container_t *>(this)->front();
    static_cast<container_t *>(this)->pop();
    _current_thread.store(_default_thread_id);
    return res;
  }

private:
  void _wait_for_ownership() {
    while (!_current_thread.compare_exchange_weak(
        _default_thread_id, std::this_thread::get_id(),
        std::memory_order_acq_rel, std::memory_order_relaxed)) {
    }
  }
};
} // namespace fsm
