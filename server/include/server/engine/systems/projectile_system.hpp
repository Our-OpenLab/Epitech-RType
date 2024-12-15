#ifndef PROJECTILE_SYSTEM_HPP_
#define PROJECTILE_SYSTEM_HPP_

#include <ecs/zipper.hpp>
#include <server/engine/game_state.hpp>

#include "server/engine/ecs_alias.hpp"

constexpr float kGameBoundaryLeft = 0.0f;
constexpr float kGameBoundaryRight = 2000.0f;
constexpr float kGameBoundaryTop = 0.0f;
constexpr float kGameBoundaryBottom = 2000.0f;

constexpr float kkMovementThresholdSquared = 0.01f * 0.01f;

inline void projectile_system(Registry& registry, const float delta_time, GameState& game_state) {
    auto& positions = registry.get_components<Position>();
    auto& velocities = registry.get_components<Velocity>();
    auto& dirty_flags = registry.get_components<DirtyFlag>();
    auto& projectiles = registry.get_components<Projectile>();

    ecs::Zipper zipper(positions, velocities, dirty_flags, projectiles);
    for (auto iter = zipper.begin(); iter != zipper.end(); ++iter) {
        auto&& [pos_opt, vel_opt, dirty_opt, projectile_opt] = *iter;
        if (!pos_opt.has_value() || !vel_opt.has_value()
            || !dirty_opt.has_value() || !projectile_opt.has_value()) {
            continue;
        }

        auto& [x, y] = *pos_opt;
        auto& [vx, vy] = *vel_opt;
        auto& is_dirty = dirty_opt->is_dirty;

        // Mettre à jour la position en fonction de la vélocité actuelle
        const float old_x = x;
        const float old_y = y;

        x += vx * delta_time;
        y += vy * delta_time;

        // Vérifier si le projectile s'est déplacé au-delà du seuil
        if ((x - old_x) * (x - old_x) +
            (y - old_y) * (y - old_y) > kkMovementThresholdSquared) {
            is_dirty = true;
        }

        // Supprimer les projectiles hors des limites
        if (x < kGameBoundaryLeft || x > kGameBoundaryRight ||
            y < kGameBoundaryTop || y > kGameBoundaryBottom) {
            const auto& [owner_id, projectile_id] = *projectile_opt;
            game_state.RemoveProjectile(projectile_id);

            std::cout << "[ProjectileSystem] Removed projectile ID "
                      << static_cast<int>(projectile_id)
                      << " at position (" << x << ", " << y << ").\n";
        }
    }
}

#endif  // PROJECTILE_SYSTEM_HPP_
