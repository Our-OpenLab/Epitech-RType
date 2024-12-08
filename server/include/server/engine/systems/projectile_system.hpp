#ifndef PROJECTILE_SYSTEM_HPP_
#define PROJECTILE_SYSTEM_HPP_

#include <ecs/zipper.hpp>
#include <cmath>
#include "server/engine/ecs_alias.hpp"

// Limites du jeu pour une zone jouable de 1920x1080
constexpr float kGameBoundaryLeft = 100.0f;
constexpr float kGameBoundaryRight = 1920.0f;  // Largeur totale de la zone jouable
constexpr float kGameBoundaryTop = 100.0f;
constexpr float kGameBoundaryBottom = 1080.0f;  // Hauteur totale de la zone jouable

constexpr float kkMovementThresholdSquared = 0.01f * 0.01f;

inline void projectile_system(Registry& registry, const float delta_time) {
    auto& positions = registry.get_components<Position>();
    auto& velocities = registry.get_components<Velocity>();
    auto& dirty_flags = registry.get_components<DirtyFlag>();

  ecs::Zipper zipper(positions, velocities, dirty_flags);
  for (auto iter = zipper.begin(); iter != zipper.end(); ++iter) {
    auto&& [pos_opt, vel_opt, dirty_opt] = *iter;
    if (!pos_opt.has_value() || !vel_opt.has_value() || !dirty_opt.has_value()) {
      continue;
    }

        auto& position = *pos_opt;
        const auto& velocity = *vel_opt;
        auto& is_dirty = dirty_opt->is_dirty;

        // Mettre à jour la position en fonction de la vitesse
        const float old_x = position.x;
        const float old_y = position.y;

        position.x += velocity.vx * delta_time;
        position.y += velocity.vy * delta_time;

        // Marquer le projectile comme "dirty" si la position a changé
        if ((position.x - old_x) * (position.x - old_x) +
            (position.y - old_y) * (position.y - old_y) > kkMovementThresholdSquared) {
            is_dirty = true;
        }

        // Supprimer les projectiles qui sortent des limites du jeu
        if (position.x < kGameBoundaryLeft || position.x > kGameBoundaryRight ||
            position.y < kGameBoundaryTop || position.y > kGameBoundaryBottom) {
            // Supprimer l'entité
            registry.kill_entity(iter.get_index());
            std::cout << "[ProjectileSystem] Removed projectile out of bounds at position ("
                      << position.x << ", " << position.y << ").\n";
        }
    }
}

#endif  // PROJECTILE_SYSTEM_HPP_
