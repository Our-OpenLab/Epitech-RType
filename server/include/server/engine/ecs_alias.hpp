#ifndef ECS_ALIAS_HPP_
#define ECS_ALIAS_HPP_

#include <ecs/registry.hpp>
#include <shared/components.hpp>

using Registry = ecs::Registry<Position, Velocity, Health, Player, Collider, Actions, DirtyFlag, Projectile, LastShotTime, AIState, PatrolPath, Aggro, Flocking, Enemy, Target>;

#endif // ECS_ALIAS_HPP_
