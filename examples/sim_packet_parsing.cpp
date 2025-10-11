#include "../examples/include/LoadBalancer.hpp"
#include "../examples/include/PacketPoller.hpp"
#include "../include/FiniteStateMachine.hpp"
#include <chrono>
#include <future>
#include <iostream>

int main([[maybe_unused]] int argc, [[maybe_unused]] const char **argv) {
  auto poller = SimPacketPoller();
  auto balancer = SimLoadBalancer<3>();
  poller.hook("Balancer", balancer);

  for (auto i = 0; i < 100'000; ++i) {
    poller.process();
    // balancer.process(poller.get());
  }
}
