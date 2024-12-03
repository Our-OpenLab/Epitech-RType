#include <iostream>
#include "ecs/registry.hpp"

struct position {
  float x, y;
};

struct velocity {
  float vx, vy;
};

int main() {
  ecs::Registry<position, velocity> reg;

  reg.register_component<position>();
  reg.register_component<velocity>();

  const auto entity1 = reg.spawn_entity();
  const auto entity2 = reg.spawn_entity();

  reg.add_component(entity1, position{0.0f, 0.0f});
  reg.add_component(entity1, velocity{1.0f, 1.5f});
  reg.add_component(entity2, position{10.0f, 10.0f});

  auto& positions = reg.get_components<position>();
  auto& velocities = reg.get_components<velocity>();

  positions[entity1]->x += velocities[entity1]->vx;
  positions[entity1]->y += velocities[entity1]->vy;

  std::cout << "Entity 1 position: (" << positions[entity1]->x << ", " << positions[entity1]->y << ")\n";
  std::cout << "Entity 2 position: (" << positions[entity2]->x << ", " << positions[entity2]->y << ")\n";

  reg.kill_entity(entity1);

  if (positions[entity1].has_value()) {
    std::cout << "Entity 1 position still exists: (" << positions[entity1]->x << ", " << positions[entity1]->y << ")\n";
  } else {
    std::cout << "Entity 1 position has been successfully removed.\n";
  }

  if (velocities[entity1].has_value()) {
    std::cout << "Entity 1 velocity still exists: (" << velocities[entity1]->vx << ", " << velocities[entity1]->vy << ")\n";
  } else {
    std::cout << "Entity 1 velocity has been successfully removed.\n";
  }

  return 0;
}
