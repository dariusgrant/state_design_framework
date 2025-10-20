#pragma once

#include <algorithm>
#include <atomic>
#include <iostream>
#include <pthread.h>
#include <syncstream>
#include <thread>
#include <vector>

namespace fsm {
class ThreadOwnership {
private:
  static inline std::thread::id _default_thread_id = std::thread::id();
  std::atomic<std::thread::id> _owning_thread;

public:
  void wait_until_yield() {
    while (_owning_thread.load() != _default_thread_id) {
      std::this_thread::yield();
    }
    _owning_thread.store(std::this_thread::get_id());
  }

  void release_ownership() { _owning_thread.store(_default_thread_id); }
};

class ScopedThreadOwnership {
private:
  ThreadOwnership &_thread_ownership;

public:
  ScopedThreadOwnership(ThreadOwnership &thread_ownership)
      : _thread_ownership(thread_ownership) {
    _thread_ownership.wait_until_yield();
  }

  ~ScopedThreadOwnership() { _thread_ownership.release_ownership(); }
};
} // namespace fsm