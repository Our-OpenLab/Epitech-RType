#ifndef PLAYER_SHOOTING_SYSTEM_HPP_
#define PLAYER_SHOOTING_SYSTEM_HPP_

#include <chrono>
#include <ecs/registry.hpp>
#include <ecs/zipper.hpp>

#include "rtype-game/components.hpp"
#include "rtype-game/engine/ecs_alias.hpp"
#include "rtype-game/engine/game_state.hpp"

constexpr auto kShootingCooldown = std::chrono::milliseconds(200);

inline void player_shooting_system(Registry& registry, GameState& game_state, const uint64_t current_time_ms) {
    auto& actions = registry.get_components<PlayerInputState>();
    auto& positions = registry.get_components<Position>();
    auto& last_shot_times = registry.get_components<LastShotTime>();
    auto& player_data = registry.get_components<ServerPlayer>();

    ecs::Zipper zipper(actions, positions, last_shot_times, player_data);

    for (auto iter = zipper.begin(); iter != zipper.end(); ++iter) {
        auto&& [actions_opt, pos_opt, last_shot_time_opt, player_opt] = *iter;

        if (!actions_opt.has_value() || !pos_opt.has_value() ||
            !last_shot_time_opt.has_value() || !player_opt.has_value()) {
          continue;
        }

        const auto& [current_actions, dir_x, dir_y] = *actions_opt;
        const auto& [x, y] = *pos_opt;
        auto& [last_shot_time] = *last_shot_time_opt;
        const auto& player = *player_opt;

        const bool wants_to_shoot = (current_actions & static_cast<uint16_t>(PlayerAction::Shoot)) != 0;
        const bool wants_auto_shoot = (current_actions & static_cast<uint16_t>(PlayerAction::AutoShoot)) != 0;

        if (wants_to_shoot || wants_auto_shoot) {
            const auto current_time = std::chrono::milliseconds(current_time_ms);

            if (current_time - last_shot_time >= kShootingCooldown) {
                last_shot_time = current_time;

              const auto player_x = x;
              const auto player_y = y;

              const float length = std::sqrt(dir_x * dir_x + dir_y * dir_y);

              if (length > 0.01f) {
                const float norm_x = dir_x / length;
                const float norm_y = dir_y / length;

                game_state.AddProjectile(player.id, player_x, player_y, norm_x, norm_y);

                std::cout << "[MessageDispatcher] Player " << static_cast<int>(player.id)
                          << " fired a projectile with direction (" << norm_x << ", " << norm_y << ").\n";
              } else {
                std::cerr << "[MessageDispatcher][WARNING] Invalid direction vector from player "
                          << static_cast<int>(player.id) << std::endl;
              }
            }
        }
    }
}

#endif // PLAYER_SHOOTING_SYSTEM_HPP_
