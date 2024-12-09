#ifndef SERVER_ENGINE_HPP_
#define SERVER_ENGINE_HPP_

#include <ecs/registry.hpp>

#include "server/engine/ecs_alias.hpp"
#include "server/engine/game_state.hpp"

class GameEngine {
 public:
  GameEngine() = default;

  ~GameEngine() = default;

  void InitializeSystems();
  void Update(float delta_time, GameState& game_state_);

  Registry& GetRegistry() { return registry_; }

 private:
  Registry registry_{};
};

#endif  // SERVER_ENGINE_HPP_
