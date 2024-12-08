#ifndef COLLISION_SYSTEM_HPP_
#define COLLISION_SYSTEM_HPP_

#include <ecs/registry.hpp>
#include <ecs/zipper.hpp>
#include <shared/components.hpp>

#include "server/engine/ecs_alias.hpp"

inline void collision_system(Registry& registry) {
  auto& positions = registry.get_components<Position>();
  auto& colliders = registry.get_components<Collider>();
  auto& dirty_flags = registry.get_components<DirtyFlag>();

  for (ecs::Zipper zipper(positions, colliders, dirty_flags);
       auto&& [pos_opt, collider_opt, dirty_opt] : zipper) {
    if (pos_opt.has_value() && collider_opt.has_value() && dirty_opt.has_value()) {
      auto& [x, y] = *pos_opt;
      auto& collider = *collider_opt;
      auto& [is_dirty] = *dirty_opt;

      if (x < 0 || y < 0) {
        collider.is_active = false;
        is_dirty = true;
      }
    }
       }
}

#endif // COLLISION_SYSTEM_HPP_
