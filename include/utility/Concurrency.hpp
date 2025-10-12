#pragma once

#include <atomic>
#include <deque>
#include <memory>

namespace fsm {
template <class _T> struct CustomerAllocator : std::allocator<_T> {};

template <class _Obj, class _Queue = std::deque<_Obj>>
class AtomicQueue : public _Queue {
private:
  std::atomic<_Queue *> _atomic_queue_ptr;

public:
  AtomicQueue() : _Queue(), _atomic_queue_ptr(this) {}

  void push(_Obj obj) {
    _atomic_queue_ptr.load(std::memory_order_acquire)->push_back(obj);
  }
  _Obj pop() {
    auto _queue = _atomic_queue_ptr.load(std::memory_order_acquire);
    auto res = _queue->front();
    _queue->pop_front();
    return res;
  }
};
} // namespace fsm
