#ifndef MESSAGE_DISPATCHER_H_
#define MESSAGE_DISPATCHER_H_

#include <array>
#include <functional>
#include <network/protocol.hpp>
#include <network/server_connection.hpp>
#include <shared/my_packet_types.hpp>

#include <server/engine/game_state.hpp>

namespace game {
class MessageDispatcher {
 public:
  using Handler = std::function<void(network::Packet<network::MyPacketType>&, const std::shared_ptr<network::ServerConnection<network::MyPacketType>>&, GameState&)>;

  static void dispatch(network::OwnedPacket<network::MyPacketType>&& owned_packet, GameState& game_state) {
    const auto index = static_cast<size_t>(owned_packet.packet.header.type);

    if (index >= handlers_.size() || !handlers_[index]) {
      default_handler(owned_packet.packet, owned_packet.connection, game_state);
      return;
    }
    handlers_[index](owned_packet.packet, owned_packet.connection, game_state);
  }

  static void default_handler(network::Packet<network::MyPacketType>& packet, const std::shared_ptr<network::ServerConnection<network::MyPacketType>>& connection, GameState& game_state);

  static const std::array<Handler,
                          static_cast<size_t>(network::MyPacketType::kMaxTypes)>
      handlers_;
};
}  // namespace game

#endif
