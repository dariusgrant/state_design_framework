// #define NDEBUG
#include "../examples/include/PacketPoller.hpp"
#include "../include/FiniteStateMachine.hpp"

int main(int argc, const char **argv) {
  auto poller = SimPacketPoller();
  poller.start();
  for (auto i = 0; i < 10; ++i) {
    std::cout << poller.get() << "\n";
    poller.process();
  }
}