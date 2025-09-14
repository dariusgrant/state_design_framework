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
class PollState : public fsm::NonTerminalState<uint64_t> {
private:
  std::random_device rd; // No copyable - thus the rule of 3 is applied here
  std::mt19937 gen;
  std::uniform_int_distribution<uint64_t> dist;

public:
  PollState(const shared_ptr_t &obj)
      : fsm::NonTerminalState<obj_t>(obj), rd(), gen(rd()),
        dist(0, UINT64_MAX) {}
  PollState(const PollState &a) : fsm::NonTerminalState<obj_t>(a) { *this = a; }
  PollState(PollState &&a) : fsm::NonTerminalState<obj_t>(std::move(a)) {}
  PollState &operator=(const PollState &a) {
    this->_obj = a._obj;
    this->gen = a.gen;
    this->dist = a.dist;
    return *this;
  }
  PollState &operator=(PollState &&a) {
    std::swap(*this, a);
    return *this;
  }
  ~PollState() {}

public:
  void enter() { *_obj.lock() = dist(gen); }
  void exit() {}

  std::type_index transition() { return fsm::GetStateTypeIndex<PollState>(); };
};

using SimPacketPoller = fsm::FiniteStateMachine<uint64_t, PollState>;
