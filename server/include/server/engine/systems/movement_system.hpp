#ifndef MOVEMENT_SYSTEM_HPP_
#define MOVEMENT_SYSTEM_HPP_

#include <ecs/zipper.hpp>
#include <shared/player_actions.hpp>
#include <cmath>

#include "server/engine/ecs_alias.hpp"

constexpr float kArenaLeft = 0.0f;
constexpr float kArenaRight = 2000.0f;
constexpr float kArenaTop = 0.0f;
constexpr float kArenaBottom = 2000.0f;

constexpr float kDefaultMaxSpeed = 2200.0f;
constexpr float kDefaultAcceleration = 22000.0f;
constexpr float kFriction = 0.1f;
constexpr float kMovementThresholdSquared = 0.01f * 0.01f;

inline void movement_system(Registry& registry, const float delta_time) {
    using namespace player_actions;

    const float delta_acceleration = kDefaultAcceleration * delta_time;

    auto& positions = registry.get_components<Position>();
    auto& velocities = registry.get_components<Velocity>();
    auto& actions = registry.get_components<Actions>();
    auto& dirty_flags = registry.get_components<DirtyFlag>();

    for (ecs::Zipper zipper(positions, velocities, actions, dirty_flags);
         auto&& [pos_opt, vel_opt, act_opt, dirty_opt] : zipper) {
        if (!pos_opt.has_value() || !vel_opt.has_value() || !act_opt.has_value() || !dirty_opt.has_value()) {
            continue;
        }

        auto& [x, y] = *pos_opt;
        auto& [vx, vy] = *vel_opt;
        const auto& [current_actions] = *act_opt;
        auto& [is_dirty] = *dirty_opt;

        const bool move_right = current_actions & PlayerAction::MoveRight;
        const bool move_left = current_actions & PlayerAction::MoveLeft;
        const bool move_down = current_actions & PlayerAction::MoveDown;
        const bool move_up = current_actions & PlayerAction::MoveUp;

        auto apply_friction_and_handle_opposites = [&](float& velocity, bool positive_command, bool negative_command) {
            if (positive_command && negative_command) {
                velocity *= 1.0f - kFriction; // Neutraliser
            } else {
                if (velocity > 0 && !positive_command) {
                    velocity -= kFriction * velocity;
                } else if (velocity < 0 && !negative_command) {
                    velocity += kFriction * std::abs(velocity);
                }

                if (velocity * velocity < kMovementThresholdSquared) velocity = 0.0f;
            }
        };

        apply_friction_and_handle_opposites(vx, move_right, move_left);
        apply_friction_and_handle_opposites(vy, move_down, move_up);

        auto apply_acceleration = [&](float& velocity, bool move_positive, bool move_negative) {
            if (move_positive) velocity += delta_acceleration;
            if (move_negative) velocity -= delta_acceleration;
        };

        apply_acceleration(vx, move_right, move_left);
        apply_acceleration(vy, move_down, move_up);

        if (const float speed_squared = vx * vx + vy * vy;
            speed_squared > kDefaultMaxSpeed * kDefaultMaxSpeed) {
            const float scale = kDefaultMaxSpeed / std::sqrt(speed_squared);
            vx *= scale;
            vy *= scale;
        }

        const float old_x = x;
        const float old_y = y;

        x += vx * delta_time;
        y += vy * delta_time;

        // Appliquer les limites de l'arène
        if (x < kArenaLeft) {
            x = kArenaLeft;
            vx = 0.0f; // Stopper le mouvement dans cette direction
        } else if (x > kArenaRight) {
            x = kArenaRight;
            vx = 0.0f;
        }

        if (y < kArenaTop) {
            y = kArenaTop;
            vy = 0.0f;
        } else if (y > kArenaBottom) {
            y = kArenaBottom;
            vy = 0.0f;
        }

        if ((x - old_x) * (x - old_x) + (y - old_y) * (y - old_y) > kMovementThresholdSquared) {
            is_dirty = true;
        }
    }
}

#endif // MOVEMENT_SYSTEM_HPP_
