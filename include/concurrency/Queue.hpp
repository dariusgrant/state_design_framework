#pragma once

#include "Threading.hpp"
#include <list>
#include <queue>

namespace fsm {

template <class _Obj> class Queue : public std::queue<_Obj, std::list<_Obj>> {
public:
  using container_t = std::queue<_Obj, std::list<_Obj>>;

private:
  ThreadOwnership _thread_owner;

public:
  Queue() : container_t() {}

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
