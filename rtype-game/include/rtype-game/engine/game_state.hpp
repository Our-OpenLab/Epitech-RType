#ifndef GAME_STATE_HPP_
#define GAME_STATE_HPP_

#include <cstdint>
#include <ecs/registry.hpp>
#include <network/network_server.hpp>
#include <unordered_map>
#include <vector>

#include "rtype-game/engine/ecs_alias.hpp"
#include "rtype-game/my_packet_types.hpp"
#include "rtype-game/components.hpp"

class GameState {
public:
  static constexpr Registry::entity_t InvalidEntity = static_cast<Registry::entity_t>(-1);

  explicit GameState(Registry& registry, network::NetworkServer<network::MyPacketType> &network_server) : registry_{registry}, next_projectile_id_{0}, network_server_(network_server) {}

  ~GameState() = default;

  bool AddPlayer(uint8_t player_id, float x, float y);
  void RemovePlayer(uint8_t player_id);
  Registry::entity_t GetEntityByPlayerId(uint8_t player_id) const;

  void AddProjectile(uint8_t player_id, float x, float y, float dir_x,
                     float dir_y);
  void RemoveProjectile(uint8_t projectile_id);

  void AddEnemy(float x, float y, AIState::State initial_state = AIState::Idle);
  void RemoveEnemy(uint8_t enemy_id);
  void AddScoreToPlayer(uint8_t player_id, int score_to_add);

  [[nodiscard]] Registry& get_registry() const {
    return registry_;
  }

private:
  struct ProjectileData {
    uint8_t owner_id;
    Registry::entity_t entity;
  };

  Registry& registry_;
  std::unordered_map<uint8_t, Registry::entity_t> player_entities_;
  std::unordered_map<uint32_t, ProjectileData> projectile_entities_;
  uint32_t next_projectile_id_;
  std::unordered_map<uint32_t, Registry::entity_t> enemy_entities_;
  uint32_t next_enemy_id_ = 0;
  network::NetworkServer<network::MyPacketType>& network_server_;
};

#endif // GAME_STATE_HPP_
