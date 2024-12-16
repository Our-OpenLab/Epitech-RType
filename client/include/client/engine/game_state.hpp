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

  Registry::entity_t AddPlayer(uint8_t player_id, float x, float y,
                               uint16_t score);
  [[nodiscard]] Registry::entity_t GetPlayer(uint8_t player_id) const;
  void RemovePlayer(uint8_t player_id);


  void AddProjectile(uint8_t projectile_id, uint8_t owner_id, float x, float y);
  [[nodiscard]] Registry::entity_t GetProjectileEntity(uint8_t projectile_id) const;
  void RemoveProjectile(uint8_t projectile_id);

  void AddEnemy(uint8_t enemy_id, float x, float y);
  [[nodiscard]] Registry::entity_t GetEnemy(uint8_t enemy_id) const;
  void RemoveEnemy(uint8_t enemy_id);

  [[nodiscard]] Registry& GetRegistry() const { return registry_; }

  void SetLocalPlayerEntity(const Registry::entity_t entity) { local_player_entity_ = entity; }

  [[nodiscard]] Registry::entity_t GetLocalPlayerEntity() const { return local_player_entity_; }

  [[nodiscard]] Position GetLocalPlayerPosition() const {
    if (local_player_entity_ == InvalidEntity) {
      return Position{};
    }

    if (const auto& positions = registry_.get_components<Position>();
        local_player_entity_ < positions.size() && positions[local_player_entity_].has_value()) {
      const auto& position = *positions[local_player_entity_];
      std::cout << "Current position: " << position.x << " " << position.y << std::endl;
      return positions[local_player_entity_].value();
    }

    return Position{};
  }

  [[nodiscard]] uint16_t GetLocalPlayerScore() const {
    if (local_player_entity_ == InvalidEntity) {
      return 0;
    }

    if (const auto& players = registry_.get_components<ClientPlayer>();
        local_player_entity_ < players.size() && players[local_player_entity_].has_value()) {
          const auto& [id, score] = *players[local_player_entity_];
          return score;
        }
    return 0;
  }


private:
  Registry& registry_;
  std::unordered_map<uint8_t, Registry::entity_t> player_entities_;
  std::unordered_map<uint8_t, Registry::entity_t> enemy_entities_;
  Registry::entity_t local_player_entity_ = InvalidEntity;
  std::unordered_map<uint32_t, Registry::entity_t> projectile_entities_;
};

}

#endif  // GAME_STATE_HPP_
