#include "../../include/FiniteStateMachine.hpp"
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <thread>

template <size_t N> class Distribute;

template <size_t N>
using LoadBalancerNonTerminalState =
    fsm::non_terminal_state_t<std::array<std::vector<uint64_t>, N>>;

template <size_t N> class Idle : public LoadBalancerNonTerminalState<N> {
public:
  STATE_ENTER_FUNCTION_NO_OP
  STATE_EXIT_FUNCTION_NO_OP

  Idle(const typename Idle::shared_ptr_t &obj)
      : LoadBalancerNonTerminalState<N>(obj) {}

  template <typename... _Args> auto transition([[maybe_unused]] _Args... _) {
    return fsm::StateIdentity<Distribute<N>>();
  };
};

template <size_t N> class Distribute : public LoadBalancerNonTerminalState<N> {
public:
  STATE_ENTER_FUNCTION_NO_OP
  STATE_EXIT_FUNCTION_NO_OP

  Distribute(const typename Distribute::shared_ptr_t &obj)
      : LoadBalancerNonTerminalState<N>(obj) {}

  void enter(uint64_t &pkt) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    auto idx = _min_value_index();
    this->get_object()[idx].emplace_back(pkt);
    std::cout << "[Balancer] Index: " << idx
              << ", Size: " << this->get_object()[idx].size() << "\n";
  }

  template <typename... _Args> auto transition([[maybe_unused]] _Args... _) {
    return fsm::StateIdentity<Distribute<N>>();
  };

private:
  auto _min_value_index() {
    auto &cur_loads = this->get_object();
    size_t min = 0;
    for (size_t i = 1; i < cur_loads.size(); ++i) {
      if (cur_loads[min].size() > cur_loads[i].size()) {
        min = i;
      }
    }
    return min;
  }
};

template <size_t N>
class SimLoadBalancer
    : public fsm::FiniteStateMachine<std::array<std::vector<uint64_t>, N>,
                                     Idle<N>, Distribute<N>> {
public:
  std::vector<size_t> load_counts() {
    std::vector<size_t> res;
    auto obj = this->get();
    for (auto &thread_loads : obj) {
      res.push_back(thread_loads.size());
    }
    return res;
  }

  void print_load_counts() {
    std::cout << "Loads:\n";
    for (auto it = this->get().begin(); it != this->get().end(); ++it) {
      std::cout << "\t" << std::distance(this->get().begin(), it) << ": "
                << it->size() << "\n";
    }
  }
};
