#ifndef ECS_ALIAS_HPP_
#define ECS_ALIAS_HPP_

#include <ecs/registry.hpp>
#include "core/components.hpp"

using Registry = ecs::Registry<ClientPlayer, Enemy, Projectile, Position>;

#endif // ECS_ALIAS_HPP_
