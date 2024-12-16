#ifndef ENEMY_MOVEMENT_SYSTEM_HPP_
#define ENEMY_MOVEMENT_SYSTEM_HPP_

#include <ecs/zipper.hpp>
#include <cmath>
#include <shared/components.hpp>
#include <unordered_map>

// Vitesse par défaut des ennemis
constexpr float kEnemyDefaultSpeed = 200.0f;

// Distance minimale pour considérer qu'un ennemi a atteint sa cible
constexpr float kMinTargetDistance = 1.0f;

// Logique de mouvement pour les ennemis
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
        if (!pos_opt.has_value() || !vel_opt.has_value() || !ai_state_opt.has_value() || !target_opt.has_value() || !dirty_opt.has_value()) {
            continue;
        }

        auto& position = *pos_opt;
        auto& velocity = *vel_opt;
        auto& ai_state = *ai_state_opt;
        auto& target = *target_opt;
        auto& [is_dirty] = *dirty_opt;

        switch (ai_state.state) {
            case AIState::Idle:
                // L'ennemi reste immobile
                velocity.vx = 0.0f;
                velocity.vy = 0.0f;
                break;

            case AIState::Pursue: {
                // Si l'ennemi n'a pas de cible valide, chercher un joueur
                if (!target.has_target) {
                    float min_distance = std::numeric_limits<float>::max();
                    uint8_t closest_player_id = static_cast<uint8_t>(-1);

                    // Parcourir les joueurs pour trouver le plus proche
                    ecs::Zipper player_zipper(positions, player_positions);
                    for (auto player_iter = player_zipper.begin(); player_iter != player_zipper.end(); ++player_iter) {
                        auto&& [player_pos_opt, player_opt] = *player_iter;
                        if (!player_pos_opt.has_value() || !player_opt.has_value()) {
                            continue;
                        }

                        const auto& player_position = *player_pos_opt;

                        // Calculer la distance au joueur
                        const float dx = player_position.x - position.x;
                        const float dy = player_position.y - position.y;
                        const float distance_squared = dx * dx + dy * dy;

                        if (distance_squared < min_distance) {
                            min_distance = distance_squared;
                            closest_player_id = player_opt->id; // Stocker l'ID du joueur
                        }
                    }

                    // Si un joueur est trouvé, définir la cible
                    if (closest_player_id != static_cast<uint8_t>(-1)) {
                        target.target_id = closest_player_id;
                        target.has_target = true;
                    }
                }

                // Si une cible est définie, poursuivre
                if (target.has_target) {
                    // Trouver la position de la cible
                    ecs::Zipper player_zipper(positions, player_positions);
                    for (auto [player_pos_opt, player_opt] : player_zipper) {
                        if (!player_pos_opt.has_value() || !player_opt.has_value()) {

                            continue;
                        }

                        if (player_opt->id == target.target_id) {
                            const auto& target_position = *player_pos_opt;

                            // Calculer la direction vers la cible
                            const float dx = target_position.x - position.x;
                            const float dy = target_position.y - position.y;
                            const float distance = std::sqrt(dx * dx + dy * dy);

                            if (distance > kMinTargetDistance) { // Si l'ennemi n'est pas encore au contact
                                velocity.vx = (dx / distance) * kEnemyDefaultSpeed;
                                velocity.vy = (dy / distance) * kEnemyDefaultSpeed;
                            } else {
                                // L'ennemi atteint le joueur (logique d'attaque peut être ajoutée ici)
                                velocity.vx = 0.0f;
                                velocity.vy = 0.0f;
                            }
                            break;
                        }
                    }
                } else {
                    // Aucun joueur à poursuivre, rester immobile
                    velocity.vx = 0.0f;
                    velocity.vy = 0.0f;
                }
                break;
            }

            case AIState::Attack:
                // Logique d'attaque (immobile, tir ou autre action)
                velocity.vx = 0.0f;
                velocity.vy = 0.0f;
                break;

            case AIState::Flee:
                // Logique de fuite (similaire à Pursue, mais inversez la direction)
                break;

            default:
                break;
        }

        // Mise à jour de la position
        position.x += velocity.vx * delta_time;
        position.y += velocity.vy * delta_time;

        is_dirty = true;
    }
}

#endif // ENEMY_MOVEMENT_SYSTEM_HPP_
