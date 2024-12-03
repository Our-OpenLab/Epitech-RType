#include <game/game_state.hpp>
#include <network/network_server.hpp>
#include <shared/my_packet_types.hpp>
#include <shared/network_messages.hpp>
#include <shared/player_actions.hpp>
#include <stdexcept>

void GameState::add_player(uint8_t id, float x, float y, float speed) {
 // std::lock_guard<std::mutex> lock(mutex_);
  players_[id] = Player{id, x, y, speed, 0, true};
}

void GameState::remove_player(uint8_t id) {
 // std::lock_guard<std::mutex> lock(mutex_);
  players_.erase(id);
}

Player& GameState::get_player(uint8_t id) {
//  std::lock_guard<std::mutex> lock(mutex_);
  if (!players_.contains(id)) {
    throw std::runtime_error("Player not found");
  }
  return players_.at(id);
}

void GameState::update(const float delta_time, network::NetworkServer<network::MyPacketType>& network_server) {
  for (auto& [id, player] : players_) {
    const float move_distance = player.speed * delta_time;
    bool position_updated = false;

    if (player.actions & static_cast<uint16_t>(PlayerAction::MoveUp)) {
      player.y -= move_distance;
      position_updated = true;
    }
    if (player.actions & static_cast<uint16_t>(PlayerAction::MoveDown)) {
      player.y += move_distance;
      position_updated = true;
    }
    if (player.actions & static_cast<uint16_t>(PlayerAction::MoveLeft)) {
      player.x -= move_distance;
      position_updated = true;
    }
    if (player.actions & static_cast<uint16_t>(PlayerAction::MoveRight)) {
      player.x += move_distance;
      position_updated = true;
    }

    if (position_updated) {
      network::UpdatePosition update_message{
        .player_id = player.id,
        .x = player.x,
        .y = player.y
    };

      auto update_packet = network::PacketFactory<network::MyPacketType>::create_packet(
          network::MyPacketType::UpdatePosition, update_message
      );

      network_server.send_to(player.id, std::move(update_packet));

      std::cout << "[Server][INFO] Player " << static_cast<int>(player.id)
                << " moved to (" << player.x << ", " << player.y << ")\n";
    }
  }
}

std::vector<Player> GameState::get_all_players() const {
//  std::lock_guard<std::mutex> lock(mutex_);

  std::vector<Player> players;
  for (const auto& [id, player] : players_) {
    players.push_back(player);
  }
  return players;
}
