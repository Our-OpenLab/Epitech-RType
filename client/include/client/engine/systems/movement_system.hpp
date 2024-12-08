#ifndef MOVEMENT_SYSTEM_HPP_
#define MOVEMENT_SYSTEM_HPP_

#include <ecs/registry.hpp>
#include <iostream>
#include <shared/components.hpp>
#include <shared/player_actions.hpp>

namespace client {

inline void movement_system(Registry& registry, const float delta_time) {
  using namespace player_actions;

  auto& positions = registry.get_components<Position>();
  auto& actions = registry.get_components<Actions>();

  for (size_t i = 0; i < positions.size(); ++i) {
    constexpr float speed = 100.0f;
    if (!positions[i].has_value() || !actions[i].has_value()) {
      continue;
    }

    auto& [x, y] = *positions[i];
    const auto& [current_actions] = *actions[i];

    if (current_actions & PlayerAction::MoveUp) y -= speed * delta_time;
    if (current_actions & PlayerAction::MoveDown) y += speed * delta_time;
    if (current_actions & PlayerAction::MoveLeft) x -= speed * delta_time;
    if (current_actions & PlayerAction::MoveRight) x += speed * delta_time;

    std::cout << "[MovementSystem] Updated position to: (" << x << ", " << y << ")\n";
  }
}

}  // namespace client

#endif  // MOVEMENT_SYSTEM_HPP_
