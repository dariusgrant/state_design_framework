// #include "../examples/include/LoadBalancer.hpp"
#include "../examples/include/PacketPoller.hpp"
#include "../include/FiniteStateMachine.hpp"

int main(int argc, const char **argv) {
  // auto p = std::make_shared<uint64_t>();
  // std::variant<PollState> a{PollState(p)};
  // std::cout << a.valueless_by_exception();
  auto poller = SimPacketPoller(5);
  // auto balancer = SimLoadBalancer<3>();
  poller.start();
  // balancer.start();

  // for (auto i = 0; i < 10; ++i) {
  //   std::cout << poller.get() << "\n";
  //   poller.process();
  // balancer.process(std::vector{poller.get()}, balancer.load_counts());
  // balancer.print_load_counts();
  // }
}