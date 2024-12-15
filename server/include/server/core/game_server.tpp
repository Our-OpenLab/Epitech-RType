#ifndef SERVER_CORE_GAME_SERVER_TPP_
#define SERVER_CORE_GAME_SERVER_TPP_

#include <iostream>
#include <array>

#include "server/core/game_server.hpp"

namespace game {

template <typename PacketType>
GameServer<PacketType>::GameServer(const uint16_t tcp_port, const uint16_t udp_port)
    : network_server_(tcp_port, udp_port, game_state_, event_queue_),
      game_state_(game_engine_.GetRegistry(), network_server_) {}

template <typename PacketType>
GameServer<PacketType>::~GameServer() {
  Stop();
}

template <typename PacketType>
bool GameServer<PacketType>::Start() {
  if (!network_server_.Start()) {
    std::cerr << "[GameServer] Failed to start network server.\n";
    return false;
  }

  game_engine_.InitializeSystems();
  running_ = true;

  game_thread_ = std::thread([this]() { Run(); });
  std::cout << "[GameServer] Server started successfully.\n";
  return true;
}

template <typename PacketType>
void GameServer<PacketType>::Stop() {
  if (!running_) return;

  running_ = false;
  network_server_.Stop();

  if (game_thread_.joinable()) {
    game_thread_.join();
  }

  std::cout << "[GameServer] Server stopped.\n";
}

template <typename PacketType>
void GameServer<PacketType>::Run() {
  uint64_t tick_counter = 0;
  auto next_tick_time = std::chrono::steady_clock::now();

  while (running_) {
    const auto current_time = std::chrono::steady_clock::now();
    const float delta_time =
        std::chrono::duration<float>(current_time - next_tick_time).count();

    event_queue_.process();
    ProcessPackets(kMaxPacketsPerTick, kMaxPacketProcessingTime);

    game_engine_.Update(delta_time, game_state_);

    if (tick_counter % kUpdateFrequencyTicks == 0) {
      const uint32_t timestamp = static_cast<uint32_t>(
          std::chrono::duration_cast<std::chrono::milliseconds>(
              current_time.time_since_epoch())
              .count());

      SendUpdatesToClients(timestamp);
    }

    ++tick_counter;
    next_tick_time += kTickDuration;

    if (const auto sleep_time =
            next_tick_time - std::chrono::steady_clock::now();
        sleep_time > std::chrono::milliseconds(0)) {
      std::this_thread::sleep_for(sleep_time);
    } else {
      std::cerr << "[GameServer] Tick overrun by "
                << -sleep_time.count() << " ms\n";
      next_tick_time = std::chrono::steady_clock::now();
    }
  }
}

template <typename PacketType>
void GameServer<PacketType>::ProcessPackets(
    const int max_packets, const std::chrono::milliseconds max_time) {
  const auto start_time = std::chrono::steady_clock::now();
  int processed = 0;

  while (processed < max_packets) {
    auto packet_opt = network_server_.PopMessage();
    if (!packet_opt) break;

    const auto current_time = std::chrono::steady_clock::now();
    const auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(current_time -
                                                              start_time);

    if (elapsed >= max_time) break;

    try {
      network_server_.process_message(std::move(packet_opt.value()));
    } catch (const std::exception& e) {
#warning need to try without try catch
      std::cerr << "[GameServer][ERROR] Exception during message processing: " << e.what() << "\n";
    }

    ++processed;
  }
}

template <typename PacketType>
void GameServer<PacketType>::SendUpdatesToClients(const uint32_t timestamp) {
  SendPlayerUpdates(timestamp);
  SendProjectileUpdates(timestamp);
  SendEnemyUpdates(timestamp);
}

template <typename PacketType>
void GameServer<PacketType>::SendPlayerUpdates(const uint32_t timestamp) {
  auto& registry = game_engine_.GetRegistry();
  auto& positions = registry.template get_components<Position>();
  auto& players = registry.template get_components<Player>();
  auto& dirty_flags = registry.template get_components<DirtyFlag>();

  std::array<network::UpdatePlayer, kMaxUpdatesPerPacket> player_updates{};
  size_t count = 0;

  for (size_t i = 0; i < positions.size(); ++i) {
    if (!dirty_flags[i].has_value() || !dirty_flags[i]->is_dirty) {
      continue;
    }

    if (positions[i].has_value() && players[i].has_value()) {
      const auto& [x, y] = *positions[i];
      const auto& [id] = *players[i];

      player_updates[count++] = network::UpdatePlayer{id, x, y};
      dirty_flags[i]->is_dirty = false;

      if (count >= kMaxUpdatesPerPacket) {
        SendUpdatePacket(player_updates, count, PacketType::kUpdatePlayers);
        count = 0;
      }
    }
  }

  if (count > 0) {
    SendUpdatePacket(player_updates, count, PacketType::kUpdatePlayers);
  }
}

template <typename PacketType>
void GameServer<PacketType>::SendEnemyUpdates(const uint32_t timestamp) {
  auto& registry = game_engine_.GetRegistry();
  auto& positions = registry.template get_components<Position>();
  auto& enemies = registry.template get_components<Enemy>();
  //auto& dirty_flags = registry.template get_components<DirtyFlag>();

  std::array<network::UpdateEnemy, kMaxUpdatesPerPacket> enemy_updates{};
  size_t count = 0;

  for (size_t i = 0; i < positions.size(); ++i) {
   // if (!dirty_flags[i].has_value() || !dirty_flags[i]->is_dirty) {
  //    continue;
  //  }

    if (positions[i].has_value() && enemies[i].has_value()) {
      const auto& [x, y] = *positions[i];
      const auto& [id] = *enemies[i];

      enemy_updates[count++] = network::UpdateEnemy{id, x, y};
   //   dirty_flags[i]->is_dirty = false;

      if (count >= kMaxUpdatesPerPacket) {
        SendUpdatePacket(enemy_updates, count, PacketType::kUpdateEnemies);
        count = 0;
      }
    }
  }

  if (count > 0) {
    SendUpdatePacket(enemy_updates, count, PacketType::kUpdateEnemies);
  }
}

template <typename PacketType>
void GameServer<PacketType>::SendProjectileUpdates(const uint32_t timestamp) {
  auto& registry = game_engine_.GetRegistry();
  auto& positions = registry.template get_components<Position>();
  auto& projectiles = registry.template get_components<Projectile>();
  auto& dirty_flags = registry.template get_components<DirtyFlag>();

  std::array<network::UpdateProjectile, kMaxUpdatesPerPacket> projectile_updates{};
  size_t count = 0;

  for (size_t i = 0; i < positions.size(); ++i) {
    if (!dirty_flags[i].has_value() || !dirty_flags[i]->is_dirty) {
      continue;
    }

    if (positions[i].has_value() && projectiles[i].has_value()) {
      const auto& [x, y] = *positions[i];
      const auto& [owner_id, projectile_id] = *projectiles[i];

      projectile_updates[count++] = network::UpdateProjectile{
        projectile_id,  // Utilisation de l'ID du projectile
        owner_id,
        x,
        y
      };
      dirty_flags[i]->is_dirty = false;

      if (count >= kMaxUpdatesPerPacket) {
        SendUpdatePacket(projectile_updates, count, PacketType::kUpdateProjectiles);
        count = 0;
      }
    }
  }

  if (count > 0) {
    SendUpdatePacket(projectile_updates, count, PacketType::kUpdateProjectiles);
  }
}



template <typename PacketType>
template <typename T>
void GameServer<PacketType>::SendUpdatePacket(
    const std::array<T, kMaxUpdatesPerPacket>& updates,
    const size_t count,
    PacketType type) {
  auto update_packet = network::PacketFactory<PacketType>::CreatePacket(
      type, std::span(updates.data(), count));
  network_server_.BroadcastUdp(std::move(update_packet));
}


/*
template <typename PacketType>
void GameServer<PacketType>::SendUpdatePacket(
    const std::array<network::UpdatePosition, kMaxUpdatesPerPacket>& updates,
    const size_t count) {
  auto update_packet = network::PacketFactory<PacketType>::CreatePacket(
      PacketType::kUpdatePosition, std::span(updates.data(), count));
  network_server_.broadcast(update_packet);
}
*/

}  // namespace game

#endif  // SERVER_CORE_GAME_SERVER_TPP_
