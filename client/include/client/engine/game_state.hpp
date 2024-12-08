#ifndef GAME_STATE_HPP_
#define GAME_STATE_HPP_

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <ecs/registry.hpp>
#include <shared/components.hpp>

#include "client/engine/ecs_alias.hpp"

namespace client {

class GameState {
public:
  explicit GameState(Registry& registry): registry_{registry} {}

  ~GameState() = default;

  void AddPlayer(uint8_t id, float x, float y);
  Registry::entity_t GetPlayer(uint8_t id) const;
  void RemovePlayer(uint8_t id);

  void AddProjectile(uint8_t projectile_id, uint8_t owner_id, float x, float y);
  void RemoveProjectile(uint8_t projectile_id);
  void RemoveProjectiles(uint8_t owner_id);

  const std::unordered_set<uint8_t>& GetProjectilesByOwner(uint8_t owner_id) const;
  Registry::entity_t GetProjectileEntity(uint8_t projectile_id) const;

  Registry& GetRegistry() const { return registry_; }

private:
  Registry& registry_;
  std::unordered_map<uint8_t, Registry::entity_t> player_entities_; // Player ID -> Entity
  std::unordered_map<uint32_t, Registry::entity_t> projectile_entities_; // Projectile ID -> Entity
  std::unordered_map<uint8_t, std::unordered_set<uint8_t>> projectiles_by_owner_; // Owner ID -> Set of Projectile IDs
};

}

#endif  // GAME_STATE_HPP_
