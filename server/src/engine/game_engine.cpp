#include <server/engine/game_engine.hpp>
#include <server/engine/systems/collision_system.hpp>
#include <server/engine/systems/movement_system.hpp>
#include <server/engine/systems/projectile_system.hpp>
#include <server/engine/systems/enemy_movement_system.hpp>

void GameEngine::InitializeSystems() {
  registry_.register_component<Position>();
  registry_.register_component<Velocity>();
  registry_.register_component<Actions>();
  registry_.register_component<Health>();
  registry_.register_component<ServerPlayer>();
  registry_.register_component<DirtyFlag>();
  registry_.register_component<Projectile>();
  registry_.register_component<LastShotTime>();
  registry_.register_component<AIState>();
  registry_.register_component<PatrolPath>();
  registry_.register_component<Aggro>();
  registry_.register_component<Flocking>();
  registry_.register_component<Enemy>();
  registry_.register_component<Target>();

  registry_.add_system([](Registry& reg, const float dt, const std::chrono::milliseconds) {
      movement_system(reg, dt);
      enemy_movement_system(reg, dt);
  });

 // registry_.add_system([](Registry& reg, const float dt, const std::chrono::milliseconds) {
 //     projectile_system(reg, dt);
 // });

  //registry_.add_system([](ServerRegistry& reg, const float dt) {
  //    collision_system(reg, dt);
  //});
}


void GameEngine::Update(const float delta_time, GameState& game_state_) {
  registry_.run_systems(delta_time, {});
  projectile_system(registry_, delta_time, game_state_);
  collision_system(registry_, game_state_);
}
