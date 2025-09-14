#include "../examples/include/LoadBalancer.hpp"
#include "../examples/include/PacketPoller.hpp"
#include "../include/FiniteStateMachine.hpp"

int main(int argc, const char **argv) {
  // auto a = std::make_shared<std::array<std::vector<uint64_t>, 3>>();
  // Distribute<3> d(a);
  // d.enter(std::vector<uint64_t>{5}, {0, 0, 0});
  auto poller = SimPacketPoller();
  auto balancer = SimLoadBalancer<3>();
  poller.start();
  balancer.start();

  // poller.set_future_object_cb([&balancer](std::shared_future<uint64_t> t) {
  //   auto pkt = t.get();
  //   std::cout << "Balancing: " << pkt << "\n";
  //   balancer.process(std::vector<uint64_t>{pkt}, balancer.load_counts());
  // });

  for (auto i = 0; i < 10; ++i) {
    std::cout << poller.get() << "\n";
    poller.process();
    balancer.process(std::vector{poller.get()});
    balancer.print_load_counts();
  }
}