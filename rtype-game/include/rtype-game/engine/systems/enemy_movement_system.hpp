#ifndef ENEMY_MOVEMENT_SYSTEM_HPP_
#define ENEMY_MOVEMENT_SYSTEM_HPP_

#include <cmath>
#include <ecs/zipper.hpp>
#include <unordered_map>

#include "rtype-game/components.hpp"
#include "rtype-game/engine/ecs_alias.hpp"

constexpr float kEnemyDefaultSpeed = 200.0f;

constexpr float kMinTargetDistance = 1.0f;

inline void enemy_movement_system(Registry& registry, const float delta_time) {
  auto& positions = registry.get_components<Position>();
  auto& velocities = registry.get_components<Velocity>();
  auto& dirty_flags = registry.get_components<DirtyFlag>();
  auto& ai_states = registry.get_components<AIState>();
  auto& targets = registry.get_components<Target>();
  auto& player_positions = registry.get_components<ServerPlayer>();

  ecs::Zipper zipper(positions, velocities, ai_states, targets, dirty_flags);
  for (auto iter = zipper.begin(); iter != zipper.end(); ++iter) {
    auto&& [pos_opt, vel_opt, ai_state_opt, target_opt, dirty_opt] = *iter;
    if (!pos_opt.has_value() || !vel_opt.has_value() ||
        !ai_state_opt.has_value() || !target_opt.has_value() ||
        !dirty_opt.has_value()) {
      continue;
    }

    auto& position = *pos_opt;
    auto& velocity = *vel_opt;
    auto& ai_state = *ai_state_opt;
    auto& target = *target_opt;
    auto& [is_dirty] = *dirty_opt;

    switch (ai_state.state) {
      case AIState::Idle:
        velocity.vx = 0.0f;
        velocity.vy = 0.0f;
        break;

      case AIState::Pursue: {
        // Vérifier si l'ennemi a une cible valide
        if (target.has_target) {
          bool target_exists = false;

          // Vérifier si le joueur cible existe toujours dans l'ECS
          ecs::Zipper player_zipper(positions, player_positions);
          for (auto [player_pos_opt, player_opt] : player_zipper) {
            if (!player_pos_opt.has_value() || !player_opt.has_value()) {
              continue;
            }

            if (player_opt->id == target.target_id) {
              target_exists = true;
              break;
            }
          }

          // Si la cible n'existe plus, réinitialiser la cible de l'ennemi
          if (!target_exists) {
            target.has_target = false;
          }
        }

        // Si l'ennemi n'a pas de cible, chercher un nouveau joueur
        if (!target.has_target) {
          float min_distance = std::numeric_limits<float>::max();
          uint8_t closest_player_id = static_cast<uint8_t>(-1);

          ecs::Zipper player_zipper(positions, player_positions);
          for (auto player_iter = player_zipper.begin();
               player_iter != player_zipper.end(); ++player_iter) {
            auto&& [player_pos_opt, player_opt] = *player_iter;
            if (!player_pos_opt.has_value() || !player_opt.has_value()) {
              continue;
            }

            const auto& player_position = *player_pos_opt;

            const float dx = player_position.x - position.x;
            const float dy = player_position.y - position.y;
            const float distance_squared = dx * dx + dy * dy;

            if (distance_squared < min_distance) {
              min_distance = distance_squared;
              closest_player_id = player_opt->id;  // Stocker l'ID du joueur
            }
          }

          if (closest_player_id != static_cast<uint8_t>(-1)) {
            target.target_id = closest_player_id;
            target.has_target = true;
          }
        }

        // Si l'ennemi a une cible valide, le poursuivre
        if (target.has_target) {
          ecs::Zipper player_zipper(positions, player_positions);
          for (auto [player_pos_opt, player_opt] : player_zipper) {
            if (!player_pos_opt.has_value() || !player_opt.has_value()) {
              continue;
            }

            if (player_opt->id == target.target_id) {
              const auto& target_position = *player_pos_opt;

              const float dx = target_position.x - position.x;
              const float dy = target_position.y - position.y;
              const float distance = std::sqrt(dx * dx + dy * dy);

              if (distance > kMinTargetDistance) {
                velocity.vx = (dx / distance) * kEnemyDefaultSpeed;
                velocity.vy = (dy / distance) * kEnemyDefaultSpeed;
              } else {
                velocity.vx = 0.0f;
                velocity.vy = 0.0f;
              }
              break;
            }
          }
        } else {
          // Aucun joueur cible, l'ennemi reste immobile
          velocity.vx = 0.0f;
          velocity.vy = 0.0f;
        }
        break;
      }

      case AIState::Attack:
        velocity.vx = 0.0f;
        velocity.vy = 0.0f;
        break;

      case AIState::Flee:
        break;

      default:
        break;
    }

    position.x += velocity.vx * delta_time;
    position.y += velocity.vy * delta_time;

    is_dirty = true;
  }
}

#endif  // ENEMY_MOVEMENT_SYSTEM_HPP_
