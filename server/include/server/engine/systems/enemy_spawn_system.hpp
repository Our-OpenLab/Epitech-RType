#ifndef ENEMY_SPAWN_SYSTEM_HPP_
#define ENEMY_SPAWN_SYSTEM_HPP_

constexpr int kEnemiesPerPlayer = 30;
constexpr float kMapBuffer = 50.0f;

constexpr float kMapHeight = 2000.0f;
constexpr float kMapWidth = 2000.0f;

#include <ecs/registry.hpp>
#include <random>

#include "server/engine/game_state.hpp"

inline void spawn_random_enemies(GameState& game_state, const int count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> pos_dist_x(-kMapBuffer, kMapWidth + kMapBuffer);
    std::uniform_real_distribution<float> pos_dist_y(-kMapBuffer, kMapHeight + kMapBuffer);

    for (int i = 0; i < count; ++i) {
        float x = pos_dist_x(gen);
        float y = pos_dist_y(gen);

        if (x >= 0 && x <= kMapWidth && y >= 0 && y <= kMapHeight) {
            if (x < kMapWidth / 2) {
                x = -kMapBuffer;
            } else {
                x = kMapWidth + kMapBuffer;
            }
        }

        game_state.AddEnemy(x, y, AIState::Pursue);
    }
}


inline void enemy_spawn_system(Registry& registry, GameState& game_state) {
    auto& player_data = registry.get_components<ServerPlayer>();
    auto& enemy_data = registry.get_components<Enemy>();

    ecs::Zipper player_zipper(player_data);
    ecs::Zipper enemy_zipper(enemy_data);

    int player_count = 0;
    for (auto [player_opt] : player_zipper) {
        if (player_opt.has_value()) {
            ++player_count;
        }
    }

    const int required_enemy_count = player_count * kEnemiesPerPlayer;

    int current_enemy_count = 0;
    for (auto [enemy_opt] : enemy_zipper) {
        if (enemy_opt.has_value()) {
            ++current_enemy_count;
        }
    }

    if (current_enemy_count < required_enemy_count) {
        const int enemies_to_spawn = required_enemy_count - current_enemy_count;
        spawn_random_enemies(game_state, enemies_to_spawn);
    }
}

#endif // ENEMY_SPAWN_SYSTEM_HPP_
