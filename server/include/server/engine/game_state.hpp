#ifndef GAME_STATE_HPP_
#define GAME_STATE_HPP_

#include <vector>
#include <cstdint>
#include <unordered_map>
#include <ecs/registry.hpp>
#include <shared/components.hpp>
#include <server/engine/ecs_alias.hpp>

class GameState {
public:
  explicit GameState(Registry& registry) : registry_{registry}, next_projectile_id_{0} {}

  ~GameState() = default;

  bool AddPlayer(uint8_t player_id, float x, float y);

  [[nodiscard]] Player& get_player(uint8_t id) const;
  [[nodiscard]] Registry::entity_t get_entity_by_player_id(uint8_t player_id) const;

  void remove_player(uint8_t id);

  [[nodiscard]] std::vector<uint8_t> get_all_player_ids() const;

  [[nodiscard]] Registry& get_registry() const {
    return registry_;
  }

  // Gestion des projectiles
  uint32_t generate_projectile_id() {
    return next_projectile_id_++;
  }

  void add_projectile(uint32_t projectile_id, uint8_t owner_id, Registry::entity_t entity) {
    projectiles_[projectile_id] = {owner_id, entity};
  }

  [[nodiscard]] Registry::entity_t get_projectile(uint32_t projectile_id) const {
    auto it = projectiles_.find(projectile_id);
    return (it != projectiles_.end()) ? it->second.entity : static_cast<Registry::entity_t>(-1);
  }

  void remove_projectile(uint32_t projectile_id) {
    projectiles_.erase(projectile_id);
  }

  [[nodiscard]] std::vector<uint32_t> get_all_projectiles_for_owner(uint8_t owner_id) const {
    std::vector<uint32_t> result;
    for (const auto& [projectile_id, data] : projectiles_) {
      if (data.owner_id == owner_id) {
        result.push_back(projectile_id);
      }
    }
    return result;
  }

private:
  struct ProjectileData {
    uint8_t owner_id;
    Registry::entity_t entity;
  };

  Registry& registry_;
  std::unordered_map<uint8_t, Registry::entity_t> player_entities_;
  std::unordered_map<uint32_t, ProjectileData> projectiles_; // Map projectiles avec ID unique
  uint32_t next_projectile_id_; // Pour générer des IDs uniques pour les projectiles
};

#endif // GAME_STATE_HPP_
