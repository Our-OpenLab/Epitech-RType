#ifndef SERVER_ENGINE_HPP_
#define SERVER_ENGINE_HPP_

#include <ecs/registry.hpp>

#include "server/engine/ecs_alias.hpp"

class GameEngine {
 public:
  GameEngine() = default;

  ~GameEngine() = default;

  void InitializeSystems();
  void Update(float delta_time);

  Registry& GetRegistry() { return registry_; }

 private:
  Registry registry_{};
};

#endif  // SERVER_ENGINE_HPP_
