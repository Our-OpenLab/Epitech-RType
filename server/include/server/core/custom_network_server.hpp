#ifndef CUSTOM_NETWORK_SERVER_H_
#define CUSTOM_NETWORK_SERVER_H_

#include <cstdlib>  // Pour rand() et RAND_MAX
#include <network/network_server.hpp>
#include <shared/network_messages.hpp>

#include "event_queue.hpp"
#include "server/engine/game_state.hpp"

inline float random_float_simple(const float min, const float max) {
  return min + static_cast<float>(rand()) / static_cast<float>(RAND_MAX / (max - min));
}

namespace network {
template <typename PacketType>
class CustomNetworkServer final : public NetworkServer<PacketType> {
public:
  explicit CustomNetworkServer(uint16_t port, GameState& game_state, EventQueue& event_queue) : NetworkServer<PacketType>(port), game_state_(game_state), event_queue_(event_queue) {}

protected:
  void on_client_accepted(
    const std::shared_ptr<ServerConnection<PacketType>>& connection) override {
    const uint32_t client_id = connection->get_id();

      constexpr float spawn_min_x = 50.0f;
      constexpr float spawn_max_x = 750.0f;
      constexpr float spawn_min_y = 50.0f;
      constexpr float spawn_max_y = 550.0f;

      float spawn_x = 100;//random_float_simple(spawn_min_x, spawn_max_x);
      float spawn_y = 100;random_float_simple(spawn_min_y, spawn_max_y);

    event_queue_.push([this, connection, client_id, spawn_x, spawn_y]() {
        if (game_state_.AddPlayer(client_id, spawn_x, spawn_y)) {
            PlayerAssign assign_message{static_cast<uint8_t>(client_id)};
            auto assign_packet = PacketFactory<PacketType>::create_packet(
                PacketType::kPlayerAssign, assign_message);
            connection->send(std::move(assign_packet));

            //PlayerJoin join_message{static_cast<uint8_t>(client_id), spawn_x, spawn_y};
            //auto join_packet = PacketFactory<PacketType>::create_packet(
            //    PacketType::kPlayerJoin, join_message);
            //this->broadcast_to_others(connection, std::move(join_packet));

            std::cout << "[Server][INFO] Player " << client_id
                      << " successfully added and notified.\n";
        } else {
            std::cout << "[Server][WARN] Player " << client_id
                      << " could not be added (already exists). Disconnecting client.\n";
            connection->disconnect();
        }
    });

    std::cout << "[Server][INFO] Player " << client_id << " added to the event queue for processing.\n";
  }

  void on_client_disconnect(const std::shared_ptr<ServerConnection<PacketType>>& connection) override {
    uint8_t player_id = connection->get_id();

    event_queue_.push([this, connection, player_id]() {
        game_state_.RemovePlayer(player_id);
        std::cout << "[CustomServer][INFO] Player " << static_cast<int>(player_id)
                  << " removed from the game.\n";

        PlayerLeave leave_message{player_id};
        auto leave_packet = PacketFactory<PacketType>::create_packet(
            PacketType::kPlayerLeave, leave_message);
        this->broadcast_to_others(connection, std::move(leave_packet));
      });

      std::cout << "[CustomServer][INFO] Player " << static_cast<int>(player_id)
                << " scheduled for removal and notification.\n";
  }

  private:
    GameState& game_state_;
    EventQueue& event_queue_;
};
}

#endif // CUSTOM_NETWORK_SERVER_H_
