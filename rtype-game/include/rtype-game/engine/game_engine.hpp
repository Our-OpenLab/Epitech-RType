#ifndef SERVER_ENGINE_HPP_
#define SERVER_ENGINE_HPP_

#include <ecs/registry.hpp>

#include "rtype-game/engine/ecs_alias.hpp"
#include "rtype-game/engine/game_state.hpp"
#include "rtype-game/game_network_server.hpp"

class GameEngine {
 public:
  GameEngine() = default;

  ~GameEngine() = default;

  void InitializeSystems();
  void Update(
      float delta_time, GameState& game_state_,
      network::GameNetworkServer<network::MyPacketType>& network_server);

  void SendPlayerUpdates(network::GameNetworkServer<network::MyPacketType>& network_server);
  void SendProjectileUpdates(network::GameNetworkServer<network::MyPacketType>& network_server);
  void SendEnemyUpdates(network::GameNetworkServer<network::MyPacketType>& network_server);

  Registry& GetRegistry() { return registry_; }

 private:
  Registry registry_{};
};

#endif  // SERVER_ENGINE_HPP_
