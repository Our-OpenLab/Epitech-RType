#include "client/engine/game_engine.hpp"

#include <ecs/zipper.hpp>

namespace client {

void GameEngine::InitializeSystems() {
  registry_.register_component<Player>();
  registry_.register_component<Enemy>();
  registry_.register_component<Projectile>();
  registry_.register_component<Position>();

//  registry_.add_system([](Registry& registry, const float delta_time) {
//    movement_system(registry, delta_time);
//  });

  /*
  registry_.add_system([](Registry& registry, float _,
                          const std::chrono::milliseconds render_time) {
    auto& positions = registry.get_components<Position>();
    auto& histories = registry.get_components<PositionHistory>();

    for (ecs::Zipper zipper(positions, histories); auto&& [pos_opt, history_opt] : zipper) {
        if (!pos_opt.has_value() || !history_opt.has_value()) continue;

        auto& [x, y] = *pos_opt;
        auto& history = *history_opt;

        if (const auto interpolated_position = history.GetInterpolatedPosition(*pos_opt, render_time)) {
            x = interpolated_position->x;
            y = interpolated_position->y;
        }
    }
  });
  */
}

void GameEngine::Update(const float delta_time, const std::chrono::milliseconds render_time) {
  //registry_.run_systems(delta_time, render_time);
 // registry_.run_systems(delta_time);
}

}  // namespace client
