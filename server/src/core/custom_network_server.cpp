#include "server/core/custom_network_server.hpp"
#include "server/core/message_dispatcher.hpp"

namespace network {

template <typename PacketType>
CustomNetworkServer<PacketType>::CustomNetworkServer(
    uint16_t tcp_port, uint16_t udp_port, GameState& game_state, EventQueue& event_queue)
    : NetworkServer<PacketType>(tcp_port, udp_port),
      game_state_(game_state),
      event_queue_(event_queue),
      message_dispatcher_(std::make_unique<MessageDispatcher>(*this)) {}

template <typename PacketType>
void CustomNetworkServer<PacketType>::ProcessMessage(OwnedPacket<PacketType>&& owned_packet) {
  message_dispatcher_->dispatch(std::move(owned_packet));
}

template <typename PacketType>
GameState& CustomNetworkServer<PacketType>::GetGameState() const {
  return game_state_;
}

template <typename PacketType>
MessageDispatcher& CustomNetworkServer<PacketType>::GetMessageDispatcher() const {
  return *message_dispatcher_;
}

template <typename PacketType>
void CustomNetworkServer<PacketType>::OnClientAccepted(
    const std::shared_ptr<TcpServerConnection<PacketType>>& connection) {
  const uint32_t client_id = connection->GetId();
  float spawn_x = 1000, spawn_y = 1000;
  uint8_t health = 100;

  event_queue_.Push([this, connection, client_id, spawn_x, spawn_y, health]() {
    if (game_state_.AddPlayer(client_id, spawn_x, spawn_y)) {
      PlayerAssign assign_message{static_cast<uint8_t>(client_id), spawn_x, spawn_y, 0, health};
      auto assign_packet = PacketFactory<PacketType>::CreatePacket(
          PacketType::kPlayerAssign, assign_message);
      connection->Send(std::move(assign_packet));

      //game_state_.AddEnemy(300.0f, 300.0f, AIState::Pursue);

      //std::cout << "[Server][INFO] Player " << client_id << " successfully added and notified.\n";
    } else {
      //std::cout << "[Server][WARN] Player " << client_id << " could not be added (already exists). Disconnecting client.\n";
      connection->Disconnect();
    }
  });

  std::cout << "[Server][INFO] Player " << client_id
            << " added to the event queue for processing.\n";
}

template <typename PacketType>
void CustomNetworkServer<PacketType>::OnClientDisconnect(
    const std::shared_ptr<TcpServerConnection<PacketType>>& connection) {
  uint8_t player_id = connection->GetId();

  event_queue_.Push([this, player_id, connection]() {
    game_state_.RemovePlayer(player_id);
    std::cout << "[CustomServer][INFO] Player " << static_cast<int>(player_id)
              << " removed from the game.\n";

    PlayerLeave leave_message{player_id};
    auto leave_packet = PacketFactory<PacketType>::CreatePacket(
        PacketType::kPlayerLeave, leave_message);
    this->BroadcastToOthersTcp(connection, std::move(leave_packet));
  });

  std::cout << "[CustomServer][INFO] Player " << static_cast<int>(player_id)
            << " scheduled for removal and notification.\n";

  auto tcp_to_udp_it = this->tcp_to_udp_map_.find(connection);
  if (tcp_to_udp_it != this->tcp_to_udp_map_.end()) {
    const auto& udp_endpoint = tcp_to_udp_it->second;

    this->udp_to_tcp_map_.erase(udp_endpoint);

    this->tcp_to_udp_map_.erase(tcp_to_udp_it);

    // std::cout << "[Server][INFO] Removed UDP endpoint mapping for client ID "
    //           << connection->GetId() << " at " << udp_endpoint.address().to_string()
    //           << ":" << udp_endpoint.port() << std::endl;
  }
}

template class CustomNetworkServer<MyPacketType>;

}  // namespace network
