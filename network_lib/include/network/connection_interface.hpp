#ifndef CONNECTION_INTERFACE_HPP_
#define CONNECTION_INTERFACE_HPP_

#include "protocol.hpp"

namespace network {

template <typename PacketType>
class ConnectionInterface {
 public:
  virtual ~ConnectionInterface() = default;

  [[nodiscard]] virtual bool is_connected() const = 0;

  virtual void disconnect() = 0;

  virtual void send(const Packet<PacketType>& packet) = 0;
  virtual void send(Packet<PacketType>&& packet) = 0;
};
}  // namespace network

#endif  // CONNECTION_INTERFACE_HPP_
