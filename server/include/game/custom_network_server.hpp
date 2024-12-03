#ifndef CUSTOM_NETWORK_SERVER_H_
#define CUSTOM_NETWORK_SERVER_H_

#include <network/network_server.hpp>
#include <shared/network_messages.hpp>

#include "event_queue.hpp"

namespace network {
template <typename PacketType>
class CustomNetworkServer final : public NetworkServer<PacketType> {
public:
  explicit CustomNetworkServer(uint16_t port, GameState& game_state, EventQueue& event_queue) : NetworkServer<PacketType>(port), game_state_(game_state), event_queue_(event_queue) {}

protected:
  void on_client_accepted(
      const std::shared_ptr<ServerConnection<PacketType>>& connection) override {
    uint32_t client_id = connection->get_id();

    network::PlayerAssign assign_message { static_cast<uint8_t>(client_id) };

    auto assign_packet = PacketFactory<PacketType>::create_packet(
        PacketType::PlayerAssign, assign_message);

    connection->send(std::move(assign_packet));

    std::cout << "[CustomServer][INFO] Sent ID " << client_id
              << " to client after acceptance." << std::endl;

    event_queue_.push([this, client_id]() {
      game_state_.add_player(client_id, 100.0f, 100.0f, 0.1f);
      std::cout << "[CustomServer][INFO] Player " << client_id << " added to the game.\n";
    });
  }

  void on_client_disconnect(const std::shared_ptr<ServerConnection<PacketType>>& connection) override {
    uint8_t player_id = connection->get_id();

    event_queue_.push([this, player_id]() {
      game_state_.remove_player(player_id);
      std::cout << "[CustomServer][INFO] Player " << static_cast<int>(player_id) << " removed from the game.\n";
    });
  }

  private:
    GameState& game_state_;
    EventQueue& event_queue_;
};
}

#endif // CUSTOM_NETWORK_SERVER_H_
