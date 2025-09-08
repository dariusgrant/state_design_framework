#include "../../include/FiniteStateMachine.hpp"
#include <cstddef>
#include <cstdint>
#include <random>
#include <string>
#include <type_traits>

// struct SimTransportLayer {
//   enum class TransportProtocol { tcp, udp };

//   uint8_t src_ip[4];
//   uint16_t src_port;
//   uint8_t dst_ip[4];
//   uint16_t dst_port;
//   TransportProtocol transport;

//   static SimTransportLayer generate_random_flow() {
//     std::random_device rd;
//     std::mt19937 gen(rd());
//     std::uniform_int_distribution<> dist8_t(0, UINT8_MAX);
//     SimTransportLayer x;
//     x.src_ip[0] = dist8_t(gen);
//     x.src_ip[1] = dist8_t(gen);
//     x.src_ip[2] = dist8_t(gen);
//     x.src_ip[3] = dist8_t(gen);
//     x.dst_ip[0] = dist8_t(gen);
//     x.dst_ip[1] = dist8_t(gen);
//     x.dst_ip[2] = dist8_t(gen);
//     x.dst_ip[3] = dist8_t(gen);

//     std::uniform_int_distribution<> dist16_t(0, UINT16_MAX);
//     x.src_port = dist16_t(gen);
//     x.dst_port = dist16_t(gen);

//     x.transport = TransportProtocol{dist8_t(gen) % 2};
//     return x;
//   };
// };

// struct SimApplicationLayer {
//   enum class ApplicationProtocol {
//     DNS,
//     FTP,
//     HTTP,
//     SSH,
//     SMB,
//     SMTP
//   };
// };

// struct SimPacket {
//   SimTransportLayer transport_layer;
//   SimApplicationLayer application_layer;
// };
class PollState : public fsm::NonTerminalState {
private:
  std::random_device rd; // No copyable - thus the rule of 5 is applied here
  std::mt19937 gen;
  std::uniform_int_distribution<uint64_t> dist;

public:
  PollState() : fsm::NonTerminalState(), rd(), gen(rd()), dist(0, UINT64_MAX) {}
  PollState(const PollState &a) {}
  PollState &operator=(const PollState &a) { return *this; }

  void enter(uint64_t &x) { x = dist(gen); }
  void exit(const uint64_t &x) {}

  std::type_index transition(const uint64_t &x) {
    return fsm::GetStateTypeIndex<PollState>();
  };
};

using SimPacketPoller = fsm::FiniteStateMachine<uint64_t, PollState>;
