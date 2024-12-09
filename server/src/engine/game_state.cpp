#include <server/engine/game_state.hpp>
#include <shared/network_messages.hpp>
#include <shared/my_packet_types.hpp>

bool GameState::AddPlayer(const uint8_t player_id, const float x, const float y) {
  if (player_entities_.contains(player_id)) {
    std::cout << "[GameState][WARN] Player ID " << static_cast<int>(player_id)
              << " already exists. Skipping addition.\n";
    return false;
  }

  const auto entity = registry_.spawn_entity();

  registry_.emplace_component<Player>(entity, Player{player_id});
  registry_.emplace_component<Position>(entity, Position{x, y});
  registry_.emplace_component<Actions>(entity, Actions{0});
  registry_.emplace_component<Velocity>(entity, Velocity{});
  registry_.emplace_component<DirtyFlag>(entity, DirtyFlag{});
  registry_.emplace_component<LastShotTime>(entity, LastShotTime{});

  player_entities_[player_id] = entity;

  std::cout << "[GameState][INFO] Player " << static_cast<int>(player_id)
            << " added at position (" << x << ", " << y << ").\n";

  return true;
}

void GameState::RemovePlayer(const uint8_t player_id) {
  if (!player_entities_.contains(player_id)) {
    std::cout << "[GameState][WARN] Player ID " << static_cast<int>(player_id)
              << " does not exist. Skipping removal.\n";
    return;
  }

  const auto entity = player_entities_[player_id];

  registry_.kill_entity(entity);

  player_entities_.erase(player_id);

  std::cout << "[GameState][INFO] Player " << static_cast<int>(player_id)
            << " successfully removed.\n";
}

Registry::entity_t GameState::GetEntityByPlayerId(
    const uint8_t player_id) const {
  if (const auto it = player_entities_.find(player_id);
      it != player_entities_.end()) {
    return it->second;
  }

  return static_cast<Registry::entity_t>(-1);
}

void GameState::AddProjectile(const uint8_t player_id, const float x, const float y) {
  const uint8_t projectile_id = next_projectile_id_++;
  const auto projectile_entity = registry_.spawn_entity();

  registry_.emplace_component<Projectile>(projectile_entity, Projectile{player_id, projectile_id});
  registry_.emplace_component<Position>(projectile_entity, Position{x, y});
  registry_.emplace_component<Velocity>(projectile_entity, Velocity{6200.0f, 0.0f});
  registry_.emplace_component<DirtyFlag>(projectile_entity, DirtyFlag{true});

  projectile_entities_[projectile_id] = ProjectileData{player_id, projectile_entity};

  std::cout << "[GameState][INFO] Added projectile with ID " << static_cast<int>(projectile_id)
            << " for player " << static_cast<int>(player_id) << " at position ("
            << x << ", " << y << ") with velocity (" << 6200.0f << ", " << 0.0f << ").\n";
}

void GameState::RemoveProjectile(const uint8_t projectile_id) {
  if (const auto it = projectile_entities_.find(projectile_id);
      it != projectile_entities_.end()) {
    const auto projectile_entity = it->second.entity;

    registry_.kill_entity(projectile_entity);

    projectile_entities_.erase(it);

    const network::RemoveProjectile remove_message{projectile_id};
    auto remove_packet = network::PacketFactory<network::MyPacketType>::create_packet(
        network::MyPacketType::kRemoveProjectile, remove_message);
    network_server_.broadcast(remove_packet);

    std::cout << "[GameState][INFO] Removed projectile with ID "
              << static_cast<int>(projectile_id) << "\n";
      } else {
        std::cout << "[GameState][WARN] Projectile ID "
                  << static_cast<int>(projectile_id)
                  << " does not exist. Skipping removal.\n";
      }
}
