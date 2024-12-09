#ifndef GAME_STATE_HPP_
#define GAME_STATE_HPP_

#include <cstdint>
#include <ecs/registry.hpp>
#include <network/network_server.hpp>
#include <server/engine/ecs_alias.hpp>
#include <shared/components.hpp>
#include <shared/my_packet_types.hpp>
#include <unordered_map>
#include <vector>

class GameState {
public:
  explicit GameState(Registry& registry, network::NetworkServer<network::MyPacketType> &network_server) : registry_{registry}, network_server_(network_server), next_projectile_id_{0} {}

  ~GameState() = default;

  bool AddPlayer(uint8_t player_id, float x, float y);
  void RemovePlayer(uint8_t player_id);
  Registry::entity_t GetEntityByPlayerId(uint8_t player_id) const;

  void AddProjectile(uint8_t player_id, float x, float y);
  void RemoveProjectile(uint8_t projectile_id);

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

  network::NetworkServer<network::MyPacketType>& network_server_;
};

#endif // GAME_STATE_HPP_
