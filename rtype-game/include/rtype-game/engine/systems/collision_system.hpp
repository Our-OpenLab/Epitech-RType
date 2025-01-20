#ifndef COLLISION_SYSTEM_HPP_
#define COLLISION_SYSTEM_HPP_

#include <ecs/registry.hpp>
#include <ecs/zipper.hpp>

#include "rtype-game/components.hpp"
#include "rtype-game/engine/ecs_alias.hpp"

inline bool CheckCollision(const Shape& shape1, const Position& pos1,
                     const Shape& shape2, const Position& pos2) {
    return std::visit(
        [&](const auto& s1, const auto& s2) -> bool {
            return is_collision(s1, pos1.x, pos1.y, s2, pos2.x, pos2.y);
        },
        shape1, shape2);
}

template<typename T1, typename T2>
void handle_collisions(ecs::Zipper<ecs::SparseArray<Position>, ecs::SparseArray<T1>>& group1,
                       ecs::Zipper<ecs::SparseArray<Position>, ecs::SparseArray<T2>>& group2,
                       std::function<void(size_t, T1&, size_t, T2&)> on_collision) {
    for (auto iter1 = group1.begin(); iter1 != group1.end(); ++iter1) {
        auto&& [pos1_opt, data1_opt] = *iter1;
        if (!pos1_opt.has_value() || !data1_opt.has_value())
            continue;

        size_t index1 = iter1.get_index();
        auto& pos1 = *pos1_opt;
        auto& entity1 = *data1_opt;

        for (auto iter2 = group2.begin(); iter2 != group2.end(); ++iter2) {
            auto&& [pos2_opt, data2_opt] = *iter2;
            if (!pos2_opt.has_value() || !data2_opt.has_value())
                continue;

            size_t index2 = iter2.get_index();
            auto& pos2 = *pos2_opt;
            auto& entity2 = *data2_opt;

            if (CheckCollision(entity1.shape, pos1, entity2.shape, pos2)) {
                on_collision(index1, entity1, index2, entity2);
            }
        }
    }
}



inline void collision_system(Registry& registry, GameState& game_state) {
    auto& positions = registry.get_components<Position>();
    auto& projectile_data = registry.get_components<Projectile>();
    auto& enemy_data = registry.get_components<Enemy>();
    auto& player_data = registry.get_components<ServerPlayer>();

    ecs::Zipper projectile_zipper(positions, projectile_data);
    ecs::Zipper enemy_zipper(positions, enemy_data);
    ecs::Zipper player_zipper(positions, player_data);

    handle_collisions<Projectile, Enemy>(
        projectile_zipper,
        enemy_zipper,
        [&](const size_t proj_index, const Projectile& projectile,
            const size_t enemy_index, const Enemy& enemy) {
            if (const auto health_opt = registry.get_component<Health>(enemy_index)) {
                auto& [value] = *health_opt;
                value -= projectile.damage;
                if (value <= 0) {
                    game_state.RemoveEnemy(enemy.id);
                    game_state.AddScoreToPlayer(projectile.owner_id, 10);
                }
            }
            game_state.RemoveProjectile(projectile.projectile_id);
        });

    handle_collisions<Enemy, ServerPlayer>(
        enemy_zipper,
        player_zipper,
        [&](const size_t enemy_index, const Enemy& enemy,
            const size_t player_index, ServerPlayer& player) {
            player.health -= 20;
            if (player.health <= 0) {
                game_state.RemovePlayer(player.id);
            }
            game_state.RemoveEnemy(enemy.id);
        });
}


#endif // COLLISION_SYSTEM_HPP_
