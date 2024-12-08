#ifndef MOVEMENT_SYSTEM_HPP_
#define MOVEMENT_SYSTEM_HPP_

#include <ecs/zipper.hpp>
#include <shared/player_actions.hpp>

#include "server/engine/ecs_alias.hpp"

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

        // Pré-calcul des commandes utilisateur
        bool move_right = current_actions & PlayerAction::MoveRight;
        bool move_left = current_actions & PlayerAction::MoveLeft;
        bool move_down = current_actions & PlayerAction::MoveDown;
        bool move_up = current_actions & PlayerAction::MoveUp;

        // Appliquer friction et opposition
        auto apply_friction_and_handle_opposites = [&](float& velocity, bool positive_command, bool negative_command) {
            if (positive_command && negative_command) {
                velocity *= 1.0f - kFriction; // Neutraliser
            } else {
                if (velocity > 0 && !positive_command) {
                    velocity -= kFriction * velocity;
                } else if (velocity < 0 && !negative_command) {
                    velocity += kFriction * std::abs(velocity);
                }

                // Clamp si proche de zéro
                if (velocity * velocity < kMovementThresholdSquared) velocity = 0.0f;
            }
        };

        apply_friction_and_handle_opposites(vx, move_right, move_left);
        apply_friction_and_handle_opposites(vy, move_down, move_up);

        // Appliquer l'accélération
        auto apply_acceleration = [&](float& velocity, bool move_positive, bool move_negative) {
            if (move_positive) velocity += delta_acceleration;
            if (move_negative) velocity -= delta_acceleration;
        };

        apply_acceleration(vx, move_right, move_left);
        apply_acceleration(vy, move_down, move_up);

        // Limitation de la vitesse
        const float speed_squared = vx * vx + vy * vy;
        if (speed_squared > kDefaultMaxSpeed * kDefaultMaxSpeed) {
            const float scale = kDefaultMaxSpeed / std::sqrt(speed_squared);
            vx *= scale;
            vy *= scale;
        }

        // Mise à jour de la position
        const float old_x = x;
        const float old_y = y;

        x += vx * delta_time;
        y += vy * delta_time;

        // Marquer comme "dirty" si la position a changé
        if ((x - old_x) * (x - old_x) + (y - old_y) * (y - old_y) > kMovementThresholdSquared) {
            is_dirty = true;
        }
    }
}

#endif // MOVEMENT_SYSTEM_HPP_
