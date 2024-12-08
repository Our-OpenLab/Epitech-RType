#include <iostream>
#include "client/engine/game_state.hpp"

namespace client {

void GameState::AddPlayer(uint8_t id, float x, float y) {
  std::cout << "[GameState][DEBUG] Adding player " << static_cast<int>(id)
            << " at position (" << x << ", " << y << ")\n";

  auto entity = registry_.spawn_entity();
  registry_.add_component<Position>(entity, {x, y});
  registry_.add_component<Actions>(entity, {});
  registry_.add_component<PositionHistory>(entity, {});
  player_entities_[id] = entity;

  std::cout << "[GameState] Added player " << static_cast<int>(id)
            << " at position (" << x << ", " << y << ")\n";
}

Registry::entity_t GameState::GetPlayer(uint8_t id) const {
  auto it = player_entities_.find(id);
  if (it == player_entities_.end()) {
    throw std::runtime_error("Player ID not found in GameState");
  }
  return it->second;
}

void GameState::RemovePlayer(uint8_t id) {
  auto it = player_entities_.find(id);
  if (it != player_entities_.end()) {
    const auto player_entity = it->second;

    // Supprimer les projectiles associés
    RemoveProjectiles(id);

    // Supprimer l'entité du joueur
    registry_.kill_entity(player_entity);
    player_entities_.erase(id);

    std::cout << "[GameState] Removed player " << static_cast<int>(id) << "\n";
  }
}

void GameState::AddProjectile(uint8_t projectile_id, uint8_t owner_id, float x, float y) {
  // Vérifier si le projectile existe déjà
  if (projectile_entities_.find(projectile_id) != projectile_entities_.end()) {
    std::cerr << "[GameState][WARNING] Projectile with ID " << projectile_id << " already exists.\n";
    return;
  }

  // Créer une nouvelle entité pour le projectile
  auto entity = registry_.spawn_entity();

  // Ajouter les composants nécessaires
  registry_.emplace_component<Position>(entity, Position{x, y});
  registry_.emplace_component<Projectile>(entity, Projectile{owner_id, projectile_id});
  //registry_.emplace_component<DirtyFlag>(entity, DirtyFlag{false});

  // Associer le projectile à son ID dans le GameState
  projectile_entities_[projectile_id] = entity;

  std::cout << "[GameState][INFO] Added projectile with ID " << projectile_id
            << " for Owner " << static_cast<int>(owner_id) << " at position (" << x << ", " << y << ").\n";
}


void GameState::RemoveProjectile(uint8_t projectile_id) {
  auto it = projectile_entities_.find(projectile_id);
  if (it != projectile_entities_.end()) {
    auto entity = it->second;
    registry_.kill_entity(entity);
    projectile_entities_.erase(projectile_id);

    // Retirer des projectiles du propriétaire
    for (auto& [owner_id, projectiles] : projectiles_by_owner_) {
      if (projectiles.erase(projectile_id)) {
        std::cout << "[GameState] Removed projectile " << projectile_id
                  << " from owner " << static_cast<int>(owner_id) << "\n";
        break;
      }
    }
  }
}

void GameState::RemoveProjectiles(uint8_t owner_id) {
  auto it = projectiles_by_owner_.find(owner_id);
  if (it != projectiles_by_owner_.end()) {
    for (auto projectile_id : it->second) {
      auto entity = projectile_entities_.at(projectile_id);
      registry_.kill_entity(entity);
      projectile_entities_.erase(projectile_id);

      std::cout << "[GameState] Removed projectile " << projectile_id
                << " for owner " << static_cast<int>(owner_id) << "\n";
    }
    projectiles_by_owner_.erase(owner_id);
  }
}

const std::unordered_set<uint8_t>& GameState::GetProjectilesByOwner(uint8_t owner_id) const {
  static const std::unordered_set<uint8_t> empty_set;
  auto it = projectiles_by_owner_.find(owner_id);
  return (it != projectiles_by_owner_.end()) ? it->second : empty_set;
}

Registry::entity_t GameState::GetProjectileEntity(uint8_t projectile_id) const {
  auto it = projectile_entities_.find(projectile_id);
  if (it == projectile_entities_.end()) {
    return static_cast<Registry::entity_t>(-1);
  }
  return it->second;
}

}  // namespace client
