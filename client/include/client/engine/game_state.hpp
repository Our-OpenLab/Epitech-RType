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
  static constexpr Registry::entity_t InvalidEntity = static_cast<Registry::entity_t>(-1);

  explicit GameState(Registry& registry): registry_{registry} {}

  ~GameState() = default;

  Registry::entity_t AddPlayer(uint8_t player_id, float x, float y);
  Registry::entity_t GetPlayer(uint8_t player_id) const;
  void RemovePlayer(uint8_t player_id);


  void AddProjectile(uint8_t projectile_id, uint8_t owner_id, float x, float y);
  Registry::entity_t GetProjectileEntity(uint8_t projectile_id) const;
  void RemoveProjectile(uint8_t projectile_id);

  Registry& GetRegistry() const { return registry_; }

  void SetLocalPlayerEntity(const Registry::entity_t entity) { local_player_entity_ = entity; }

  Registry::entity_t GetLocalPlayerEntity() const { return local_player_entity_; }

  Position GetLocalPlayerPosition() const {
    static Position* cached_position = nullptr;
    static Registry::entity_t last_checked_entity = InvalidEntity;

    if (!cached_position || local_player_entity_ != last_checked_entity) {
      last_checked_entity = local_player_entity_;
      if (local_player_entity_ != InvalidEntity) {
        if (auto& positions = registry_.get_components<Position>();
            local_player_entity_ < positions.size() && positions[local_player_entity_].has_value()) {
          cached_position = &(*positions[local_player_entity_]);
        } else {
          cached_position = nullptr;
        }
      } else {
        cached_position = nullptr;
      }
    }

    return cached_position ? *cached_position : Position{};
  }


private:
  Registry& registry_;
  std::unordered_map<uint8_t, Registry::entity_t> player_entities_;
  Registry::entity_t local_player_entity_ = InvalidEntity;
  std::unordered_map<uint32_t, Registry::entity_t> projectile_entities_;
};

}

#endif  // GAME_STATE_HPP_
