#include "rtype-game/engine/game_engine.hpp"

#include "rtype-game/engine/systems/collision_system.hpp"
#include "rtype-game/engine/systems/enemy_movement_system.hpp"
#include "rtype-game/engine/systems/enemy_spawn_system.hpp"
#include "rtype-game/engine/systems/movement_system.hpp"
#include "rtype-game/engine/systems/player_shooting_system.hpp"

#include "rtype-game/engine/systems/projectile_system.hpp"
#include "rtype-game/protocol.hpp"
#include "rtype-game/my_packet_types.hpp"

void GameEngine::InitializeSystems() {
  registry_.register_component<Position>();
  registry_.register_component<Velocity>();
  registry_.register_component<PlayerInputState>();
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

constexpr std::size_t kMaxPacketSize = 1400;

template <typename UpdateType>
constexpr std::size_t GetMaxUpdatesPerPacket() {
  constexpr std::size_t update_size = sizeof(UpdateType);

  static_assert(update_size > 0, "UpdateType size must be greater than 0.");
  static_assert(update_size <= kMaxPacketSize, "UpdateType size exceeds maximum packet size.");

  return kMaxPacketSize / update_size;
}

void GameEngine::SendPlayerUpdates(network::GameNetworkServer<network::MyPacketType>& network_server) {
  auto& positions = registry_.get_components<Position>();
  auto& players = registry_.get_components<ServerPlayer>();
  auto& dirty_flags = registry_.get_components<DirtyFlag>();

  constexpr std::size_t MaxUpdatesPerPacket = GetMaxUpdatesPerPacket<network::packets::UpdatePlayer>();
  std::array<network::packets::UpdatePlayer, MaxUpdatesPerPacket> updates{};
  size_t count = 0;

  for (size_t i = 0; i < positions.size(); ++i) {
    if (!positions[i].has_value() || !players[i].has_value()) {
      continue;
    }
   // if (!force_update && (!dirty_flags.has(i) || !dirty_flags[i]->is_dirty)) {
   //   continue;
   // }

  //  if (!dirty_flags[i].has_value() || !dirty_flags[i]->is_dirty) {
  //    continue;
  //  }

    const auto& [x, y] = *positions[i];
    const auto& player = *players[i];
    updates[count++] = network::packets::UpdatePlayer{
      static_cast<uint8_t>(player.id),
      x,
      y,
      static_cast<uint16_t>(player.score),
      static_cast<uint8_t>(player.health)
    };

    if (dirty_flags[i].has_value()) {
      dirty_flags[i]->is_dirty = false;
    }

    if (count >= MaxUpdatesPerPacket) {
      auto packet = network::PacketFactory<network::MyPacketType>::CreatePacket(
          network::MyPacketType::kUpdatePlayers,
          std::span(updates.data(), count));
      network_server.BroadcastUdp(std::move(packet));
      count = 0;
    }
  }

  if (count > 0) {
    auto packet = network::PacketFactory<network::MyPacketType>::CreatePacket(
        network::MyPacketType::kUpdatePlayers,
        std::span(updates.data(), count));
    network_server.BroadcastUdp(std::move(packet));
  }
}

void GameEngine::SendProjectileUpdates(network::GameNetworkServer<network::MyPacketType>& network_server) {
  auto& positions = registry_.get_components<Position>();
  auto& projectiles = registry_.get_components<Projectile>();
  auto& dirty_flags = registry_.get_components<DirtyFlag>();

  constexpr std::size_t MaxUpdatesPerPacket = GetMaxUpdatesPerPacket<network::packets::UpdateProjectile>();
  std::array<network::packets::UpdateProjectile, MaxUpdatesPerPacket> updates{};
  size_t count = 0;

  for (size_t i = 0; i < positions.size(); ++i) {
    if (!positions[i].has_value() || !projectiles[i].has_value()) {
      continue;
    }

//    if (!dirty_flags[i].has_value() || !dirty_flags[i]->is_dirty) {
//      continue;
//    }

    const auto& [x, y] = *positions[i];
    const auto& [owner_id, projectile_id, unused1, unused2] = *projectiles[i];

    updates[count++] = network::packets::UpdateProjectile{
        projectile_id,
        owner_id,
        x,
        y
    };

    dirty_flags[i]->is_dirty = false;

    if (count >= MaxUpdatesPerPacket) {
      auto packet = network::PacketFactory<network::MyPacketType>::CreatePacket(
          network::MyPacketType::kUpdateProjectiles,
          std::span(updates.data(), count));
      network_server.BroadcastUdp(std::move(packet));
      count = 0;
    }
  }

  if (count > 0) {
    auto packet = network::PacketFactory<network::MyPacketType>::CreatePacket(
        network::MyPacketType::kUpdateProjectiles,
        std::span(updates.data(), count));
    network_server.BroadcastUdp(std::move(packet));
  }
}

void GameEngine::SendEnemyUpdates(network::GameNetworkServer<network::MyPacketType>& network_server) {
    auto& positions = registry_.get_components<Position>();
    auto& enemies = registry_.get_components<Enemy>();
    auto& dirty_flags = registry_.get_components<DirtyFlag>();

    constexpr std::size_t MaxUpdatesPerPacket = GetMaxUpdatesPerPacket<network::packets::UpdateEnemy>();
    std::array<network::packets::UpdateEnemy, MaxUpdatesPerPacket> updates{};
    size_t count = 0;

    for (size_t i = 0; i < positions.size(); ++i) {
        if (!positions[i].has_value() || !enemies[i].has_value()) {
          continue;
        }

    //    if (!dirty_flags[i].has_value() || !dirty_flags[i]->is_dirty) {
    //        continue;
    //    }

        const auto& [x, y] = *positions[i];
        const auto& [id, shape] = *enemies[i];

        updates[count++] = network::packets::UpdateEnemy{
          id,
          x,
          y
        };

        if (dirty_flags[i].has_value()) {
          dirty_flags[i]->is_dirty = false;
        }

        if (count >= MaxUpdatesPerPacket) {
          auto packet = network::PacketFactory<network::MyPacketType>::CreatePacket(
              network::MyPacketType::kUpdateEnemies,
              std::span(updates.data(), count));
          network_server.BroadcastUdp(std::move(packet));
          count = 0;
        }
    }

    if (count > 0) {
        auto packet = network::PacketFactory<network::MyPacketType>::CreatePacket(
            network::MyPacketType::kUpdateEnemies,
            std::span(updates.data(), count));
        network_server.BroadcastUdp(std::move(packet));
    }
}

void GameEngine::Update(const float delta_time, GameState& game_state_, network::GameNetworkServer<network::MyPacketType>& network_server) {
  registry_.run_systems(delta_time, {});
  projectile_system(registry_, delta_time, game_state_);
  collision_system(registry_, game_state_);
  enemy_spawn_system(registry_, game_state_);

  const auto current_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::steady_clock::now().time_since_epoch()).count();

  player_shooting_system(registry_, game_state_, current_time_ms);

  SendPlayerUpdates(network_server);
  SendProjectileUpdates(network_server);
  SendEnemyUpdates(network_server);
}
