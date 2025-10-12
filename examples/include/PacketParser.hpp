#include "../../include/FiniteStateMachine.hpp"
#include <cstddef>
#include <cstdint>
#include <random>
#include <string>
#include <type_traits>

using ParserNonTerminalState = fsm::NonTerminalState<uint64_t>;
using ParserTerminalState = fsm::TerminalState<uint64_t>;

class ByteParser : public ParserNonTerminalState {};

template <size_t N>
using LoadBalancerNonTerminalState =
    fsm::NonTerminalState<std::array<std::vector<uint64_t>, N>>;

template <size_t N> class Idle : public LoadBalancerNonTerminalState<N> {
public:
  Idle(const typename Idle::shared_ptr_t &obj)
      : LoadBalancerNonTerminalState<N>(obj) {}

  template <typename... _Args> auto transition(_Args... _) {
    return fsm::StateIdentity<Distribute<N>>();
  };
};

template <size_t N> class Distribute : public LoadBalancerNonTerminalState<N> {
public:
  using LoadBalancerNonTerminalState<N>::enter;

  Distribute(const typename Distribute::shared_ptr_t &obj)
      : LoadBalancerNonTerminalState<N>(obj) {}

  void enter(std::vector<uint64_t> pkts) {
    for (auto p : pkts) {
      std::cout << "Balancing packet: " << p << "\n";
      auto min = _min_value_index();
      std::cout << "Pushing load on idx: " << min << "\n";
      this->get_object()[min].push_back(p);
    }
  }

  template <typename... _Args> auto transition(_Args... _) {
    return fsm::StateIdentity<Distribute<N>>();
  };

private:
  auto _min_value_index() {
    auto cur_loads = this->get_object();
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
