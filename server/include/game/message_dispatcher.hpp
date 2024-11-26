#ifndef MESSAGE_DISPATCHER_H_
#define MESSAGE_DISPATCHER_H_

#include <array>
#include <functional>

#include <network/protocol.hpp>
#include <network/server_connection.hpp>

namespace game {
class MessageDispatcher {
 public:
  using Handler = std::function<void(network::Packet&, const std::shared_ptr<network::ServerConnection>&)>;

  static void dispatch(network::OwnedPacket&& owned_packet) {
    const auto index = static_cast<size_t>(owned_packet.packet.header.type);

    if (index >= handlers_.size() || !handlers_[index]) {
      default_handler(owned_packet.packet, owned_packet.connection);
      return;
    }
    handlers_[index](owned_packet.packet, owned_packet.connection);
  }

  static void default_handler(network::Packet& packet, const std::shared_ptr<network::ServerConnection>& connection);

  static const std::array<Handler,
                          static_cast<size_t>(network::PacketType::MaxTypes)>
      handlers_;
};
}  // namespace game

#endif
