#include "../examples/include/Counter.hpp"
#include "../include/FiniteStateMachine.hpp"
#include <iostream>

using namespace std::chrono_literals;

int main(int argc, const char **argv) {
  auto c1 = fsm::FiniteStateMachine<int, Counter, MaxCounter>();
  c1.start(1);
  while (!c1.is_terminated()) {
    c1.process(1);
    std::cout << c1.get() << "\n";
  }
}