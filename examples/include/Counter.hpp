
#pragma once
#include "../../include/AbstractState.hpp"
#include "../../include/utility/StateType.hpp"
#include <iostream>
#include <limits>

class MaxCounter;

class Counter : public fsm::NonTerminalState {
public:
  inline static const auto TI = fsm::GetStateTypeIndex<Counter>();
  template <class IntType> void enter(int &x, IntType n) {
    static_assert(std::is_integral_v<IntType>);
    if (size_t(x) + n >=
        std::numeric_limits<std::remove_reference_t<decltype(x)>>::max()) {
      x = std::numeric_limits<std::remove_reference_t<decltype(x)>>::max();
    } else {
      x += n;
    }
  }
  void exit(const int &x, int n) {}

  std::type_index transition(const int &x, int a) {
    if (!(x ==
          std::numeric_limits<std::remove_reference_t<decltype(x)>>::max())) {
      return fsm::GetStateTypeIndex<Counter>();
    } else {
      return fsm::GetStateTypeIndex<MaxCounter>();
    }
  };
};

class MaxCounter : public fsm::TerminalState {
public:
  template <class IntType> void enter(int &x, IntType n) {
    static_assert(std::is_integral_v<IntType>);
    std::cout << "Counter at max: " << x << "\n";
  }
  void exit(const int &x, int n) {}

  std::type_index transition(const int &x, int a) {
    return fsm::GetStateTypeIndex<MaxCounter>();
  };
};
