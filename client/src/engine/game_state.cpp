#include <iostream>
#include "client/engine/game_state.hpp"

namespace client {

bool GameState::AddPlayer(const uint8_t player_id, const float x, const float y) {
  if (player_entities_.contains(player_id)) {
    std::cout << "[GameState][WARN] Player ID " << static_cast<int>(player_id)
              << " already exists. Skipping addition.\n";
    return false;
  }

  const auto entity = registry_.spawn_entity();

  registry_.add_component<Position>(entity, {x, y});
  registry_.add_component<Player>(entity, Player{player_id});
  registry_.add_component<DirtyFlag>(entity, DirtyFlag{true});

  player_entities_[player_id] = entity;

  std::cout << "[GameState][INFO] Added player " << static_cast<int>(player_id)
            << " at position (" << x << ", " << y << ").\n";

  return true;
}

Registry::entity_t GameState::GetPlayer(const uint8_t player_id) const {
  const auto it = player_entities_.find(player_id);
  if (it == player_entities_.end()) {
    std::cout << "[GameState][WARN] Player ID " << static_cast<int>(player_id)
              << " not found in GameState.\n";
    return InvalidEntity;
  }
  return it->second;
}

void GameState::RemovePlayer(const uint8_t player_id) {
  const auto it = player_entities_.find(player_id);
  if (it == player_entities_.end()) {
    std::cout << "[GameState][WARN] Player ID " << static_cast<int>(player_id)
              << " not found. Skipping removal.\n";
    return;
  }

  const auto player_entity = it->second;

  registry_.kill_entity(player_entity);
  player_entities_.erase(player_id);

  std::cout << "[GameState][INFO] Removed player " << static_cast<int>(player_id) << "\n";
}

void GameState::AddProjectile(const uint8_t projectile_id,
                              const uint8_t owner_id,
                              const float x,
                              const float y) {
  if (projectile_entities_.contains(projectile_id)) {
    std::cerr << "[GameState][WARNING] Projectile with ID " << projectile_id
              << " already exists.\n";
    return;
  }

  const auto entity = registry_.spawn_entity();

  registry_.emplace_component<Position>(entity, Position{x, y});
  registry_.emplace_component<Projectile>(entity, Projectile{owner_id, projectile_id});

  projectile_entities_[projectile_id] = entity;

  std::cout << "[GameState][INFO] Added projectile with ID " << projectile_id
            << " for Owner " << static_cast<int>(owner_id) << " at position (" << x << ", " << y << ").\n";
}

Registry::entity_t GameState::GetProjectileEntity(
    const uint8_t projectile_id) const {
  const auto it = projectile_entities_.find(projectile_id);
  if (it == projectile_entities_.end()) {
    return static_cast<Registry::entity_t>(-1);
  }
  return it->second;
}

void GameState::RemoveProjectile(const uint8_t projectile_id) {
  const auto it = projectile_entities_.find(projectile_id);
  if (it == projectile_entities_.end()) {
    std::cerr << "[GameState][WARN] Projectile ID " << static_cast<int>(projectile_id)
              << " not found. Skipping removal.\n";
    return;
  }

  const auto projectile_entity = it->second;

  registry_.kill_entity(projectile_entity);

  projectile_entities_.erase(projectile_id);

  std::cout << "[GameState][INFO] Removed projectile with ID " << static_cast<int>(projectile_id) << "\n";
}


}  // namespace client
