#pragma once

#include <atomic>
#include <list>
#include <queue>
#include <thread>

namespace fsm {
class ThreadOwnership {
private:
  static inline std::thread::id _default_thread_id = std::thread::id();
  std::atomic<std::thread::id> _owning_thread;

public:
  void wait_until_yield(
      std::thread::id requesting_thread = std::this_thread::get_id()) {
    while (!_owning_thread.compare_exchange_weak(
        _default_thread_id, requesting_thread, std::memory_order_acq_rel,
        std::memory_order_relaxed)) {
    }
  }

  void release_ownership() { _owning_thread.store(_default_thread_id); }
};

class ScopedThreadOwnership {
private:
  ThreadOwnership &_thread_ownership;

public:
  ScopedThreadOwnership(
      ThreadOwnership &thread_ownership,
      std::thread::id requesting_thread = std::this_thread::get_id())
      : _thread_ownership(thread_ownership) {
    _thread_ownership.wait_until_yield(requesting_thread);
  }

  ~ScopedThreadOwnership() { _thread_ownership.release_ownership(); }
};

template <class _Obj>
class AtomicQueue : public std::queue<_Obj, std::list<_Obj>> {
public:
  using container_t = std::queue<_Obj, std::list<_Obj>>;

private:
  ThreadOwnership _thread_owner;

public:
  AtomicQueue() : container_t() {}

  void push(_Obj obj) {
    auto scoped_thread_ownership = ScopedThreadOwnership(_thread_owner);
    static_cast<container_t *>(this)->push(obj);
  }

  _Obj pop() {
    auto scoped_thread_ownership = ScopedThreadOwnership(_thread_owner);
    auto res = static_cast<container_t *>(this)->front();
    static_cast<container_t *>(this)->pop();
    return res;
  }
};
} // namespace fsm
