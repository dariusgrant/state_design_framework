#pragma once
#include <functional>
#include <future>
#include <unordered_map>

namespace fsm {
// enum struct SubscriptionType : uint8_t {
//   None = 0x0,
//   Subset = 0x1,
//   Conditional = 0x2,
//   All = 0xF
// };

// struct SubscriptionPolicy {
//   SubscriptionType type;
//   std::tuple<> subset_types;

// };

template <typename _Obj>
struct FutureSubscription
    : public std::unordered_map<std::string,
                                std::function<void(std::shared_future<_Obj>)>> {
  template <class _Fsm> void add(std::string name, _Fsm &fsm) {
    this->emplace(
        name, [&f = fsm](std::shared_future<_Obj> sf) { f.process(sf.get()); });
  }

  void add(std::string name,
           std::function<void(std::shared_future<_Obj>)> callback) {
    this->emplace(
        name, [&cb = callback](std::shared_future<_Obj> sf) { cb(sf.get()); });
  }

  void remove(std::string name) { this->erase(name); }
};
} // namespace fsm
