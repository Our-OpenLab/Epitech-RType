#ifndef ECS_ALIAS_HPP_
#define ECS_ALIAS_HPP_

#include <ecs/registry.hpp>
#include <shared/components.hpp>

using Registry = ecs::Registry<Player, Enemy, Projectile, Position>;

#endif // ECS_ALIAS_HPP_
