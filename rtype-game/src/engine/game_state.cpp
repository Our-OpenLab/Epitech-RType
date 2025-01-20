#include "rtype-game/engine/game_state.hpp"

#include "rtype-game/my_packet_types.hpp"
#include "rtype-game/network_messages.hpp"
#include "rtype-game/protocol.hpp"

bool GameState::AddPlayer(const uint8_t player_id, const float x, const float y) {
  if (player_entities_.contains(player_id)) {
    std::cout << "[GameState][WARN] Player ID " << static_cast<int>(player_id)
              << " already exists. Skipping addition.\n";
    return false;
  }
  try {
    const auto entity = registry_.spawn_entity();

    registry_.emplace_component<ServerPlayer>(entity, ServerPlayer{player_id, Circle{30.0f}});
    registry_.emplace_component<Position>(entity, Position{x, y});
    registry_.emplace_component<PlayerInputState>(entity, PlayerInputState{});
    registry_.emplace_component<Velocity>(entity, Velocity{});
    registry_.emplace_component<DirtyFlag>(entity, DirtyFlag{});
    registry_.emplace_component<LastShotTime>(entity, LastShotTime{});

    player_entities_[player_id] = entity;

    std::cout << "[GameState][INFO] Player " << static_cast<int>(player_id)
                    << " added at position (" << x << ", " << y << ").\n";

    return true;
  } catch (const std::exception& e) {
    std::cerr << "[GameState][ERROR] Failed to add player " << static_cast<int>(player_id)
              << ": " << e.what() << '\n';
    return false;
  }
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

  const network::packets::RemovePlayer remove_message{player_id};
  auto remove_packet = network::PacketFactory<network::MyPacketType>::CreatePacket(
      network::MyPacketType::kRemovePlayer, remove_message);
  network_server_.BroadcastTcp(remove_packet);

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

void GameState::AddProjectile(const uint8_t player_id, const float x, const float y,
                              const float dir_x, const float dir_y) {
  const uint8_t projectile_id = next_projectile_id_++;
  const auto projectile_entity = registry_.spawn_entity();

  constexpr float projectile_speed = 1240.0f;
  const float velocity_x = dir_x * projectile_speed;
  const float velocity_y = dir_y * projectile_speed;

  registry_.emplace_component<Projectile>(projectile_entity, Projectile{player_id, projectile_id, Circle{5.0f}, 50});
  registry_.emplace_component<Position>(projectile_entity, Position{x, y});
  registry_.emplace_component<Velocity>(projectile_entity, Velocity{velocity_x, velocity_y});
  registry_.emplace_component<DirtyFlag>(projectile_entity, DirtyFlag{true});

  projectile_entities_[projectile_id] = ProjectileData{player_id, projectile_entity};

  std::cout << "[GameState][INFO] Added projectile with ID " << static_cast<int>(projectile_id)
            << " for player " << static_cast<int>(player_id) << " at position ("
            << x << ", " << y << ") with velocity (" << dir_x << ", " << dir_y << ").\n";
}

void GameState::RemoveProjectile(const uint8_t projectile_id) {
  if (const auto it = projectile_entities_.find(projectile_id);
      it != projectile_entities_.end()) {
    const auto projectile_entity = it->second.entity;

    registry_.kill_entity(projectile_entity);

    projectile_entities_.erase(it);

    const network::RemoveProjectile remove_message{projectile_id};
    auto remove_packet = network::PacketFactory<network::MyPacketType>::CreatePacket(
        network::MyPacketType::kRemoveProjectile, remove_message);
    network_server_.BroadcastTcp(remove_packet);

    std::cout << "[GameState][INFO] Removed projectile with ID " << static_cast<int>(projectile_id) << "\n";
      } else {
        std::cout << "[GameState][WARN] Projectile ID " << static_cast<int>(projectile_id)
                  << " does not exist. Skipping removal.\n";
      }
}

void GameState::AddEnemy(const float x, const float y,
                         const AIState::State initial_state) {
  const uint8_t enemy_id = next_enemy_id_++;
  const auto enemy_entity = registry_.spawn_entity();

  registry_.emplace_component<Enemy>(enemy_entity, Enemy{enemy_id, Circle{30.0f}});
  registry_.emplace_component<Position>(enemy_entity, Position{x, y});
  registry_.emplace_component<AIState>(enemy_entity, AIState{initial_state});
  registry_.emplace_component<Target>(enemy_entity, Target{});
  registry_.emplace_component<DirtyFlag>(enemy_entity, DirtyFlag{});
  registry_.emplace_component<Velocity>(enemy_entity, Velocity{0.0f, 0.0f});
  registry_.emplace_component<Health>(enemy_entity, Health{100});

  if (initial_state == AIState::Patrol) {
    PatrolPath patrol_path;
    patrol_path.waypoints = {
      {x + 100.0f, y},
      {x, y + 100.0f},
      {x - 100.0f, y},
      {x, y - 100.0f}
    };
    patrol_path.loop = true;
    registry_.emplace_component<PatrolPath>(enemy_entity, std::move(patrol_path));
  }

  enemy_entities_[enemy_id] = enemy_entity;

  std::cout << "[GameState][INFO] Added enemy with ID " << enemy_id
            << " at position (" << x << ", " << y << ").\n";
}

void GameState::RemoveEnemy(const uint8_t enemy_id) {
  if (const auto it = enemy_entities_.find(enemy_id); it != enemy_entities_.end()) {
    const auto enemy_entity = it->second;

    registry_.kill_entity(enemy_entity);

    enemy_entities_.erase(it);

    const network::RemoveEnemy remove_message{enemy_id};
    auto remove_packet = network::PacketFactory<network::MyPacketType>::CreatePacket(
        network::MyPacketType::kRemoveEnemy, remove_message);
    network_server_.BroadcastTcp(remove_packet);

    std::cout << "[GameState][INFO] Removed enemy with ID " << enemy_id << ".\n";
  } else {
    std::cout << "[GameState][WARN] Enemy ID " << enemy_id
              << " does not exist. Skipping removal.\n";
  }
}

void GameState::AddScoreToPlayer(const uint8_t player_id,
                                 const int score_to_add) {
  if (player_entities_.contains(player_id)) {
    const auto entity_id = player_entities_[player_id];

    if (const auto player_opt = registry_.get_component<ServerPlayer>(entity_id)) {
      auto& player = *player_opt;
      player.score += score_to_add;

      std::cout << "Player " << static_cast<int>(player.id)
                << " scored! New score: " << player.score << std::endl;
    }
  } else {
    std::cerr << "Player ID " << static_cast<int>(player_id)
              << " not found in player_entities_!" << std::endl;
  }
}
