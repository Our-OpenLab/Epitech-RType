#ifndef GAME_ENGINE_HPP_
#define GAME_ENGINE_HPP_

#include <ecs/registry.hpp>
#include <shared/components.hpp>
#include <functional>
#include <vector>

#include "client/engine/ecs_alias.hpp"

namespace client {

class GameEngine {
public:
  GameEngine() = default;
  ~GameEngine() = default;

  void InitializeSystems();
  void Update(float delta_time, std::chrono::milliseconds render_time);

  Registry& GetRegistry() { return registry_; }

private:
  Registry registry_;
};

}  // namespace client

#endif  // GAME_ENGINE_HPP_
