#ifndef MESSAGE_DISPATCHER_HPP_
#define MESSAGE_DISPATCHER_HPP_

#include <array>
#include <client/engine/game_state.hpp>
#include <functional>
#include <network/protocol.hpp>
#include <shared/my_packet_types.hpp>
#include <shared/network_messages.hpp>

#include "client.hpp"

namespace network {

class MessageDispatcher {
public:
  using Handler = std::function<void(Packet<MyPacketType>&)>;

  explicit MessageDispatcher(Client& client)
      : client_(client) {
    InitializeHandlers();
  }

  void Dispatch(Packet<MyPacketType>&& packet) const {
    const auto index = static_cast<size_t>(packet.header.type);

    if (index >= handlers_.size() || !handlers_[index]) {
      DefaultHandler(std::move(packet));
      return;
    }

    handlers_[index](packet);
  }

private:
  void InitializeHandlers() {
    handlers_.fill(nullptr);

    handlers_[static_cast<size_t>(MyPacketType::kPlayerAssign)] = [this](const Packet<MyPacketType>& packet) {
      HandlePlayerAssign(packet);
    };

    handlers_[static_cast<size_t>(MyPacketType::kUpdatePlayers)] = [this](const Packet<MyPacketType>& packet) {
      HandleUpdatePlayers(packet);
    };

    handlers_[static_cast<size_t>(MyPacketType::kUpdateEnemies)] = [this](const Packet<MyPacketType>& packet) {
      HandleUpdateEnemies(packet);
    };

    handlers_[static_cast<size_t>(MyPacketType::kUpdateProjectiles)] = [this](const Packet<MyPacketType>& packet) {
      HandleUpdateProjectiles(packet);
    };

    handlers_[static_cast<size_t>(MyPacketType::kRemoveProjectile)] = [this](const Packet<MyPacketType>& packet) {
      HandleRemoveProjectile(packet);
    };

    handlers_[static_cast<size_t>(MyPacketType::kPlayerJoin)] = [this](const Packet<MyPacketType>& packet) {
      HandlePlayerJoin(packet);
    };

    handlers_[static_cast<size_t>(MyPacketType::kPlayerLeave)] = [this](const Packet<MyPacketType>& packet) {
      HandlePlayerLeave(packet);
    };

    handlers_[static_cast<size_t>(MyPacketType::kRemoveEnemy)] = [this](const Packet<MyPacketType>& packet) {
      HandleRemoveEnemy(packet);
    };
  }

  static void DefaultHandler(Packet<MyPacketType>&& packet) {
    std::cerr << "[MessageDispatcher][WARNING] Unhandled packet "
              << packet << std::endl;
  }

  void HandlePlayerAssign(const Packet<MyPacketType>& packet) const {
    const auto [player_id, spawn_x, spawn_y, score] =
        PacketFactory<MyPacketType>::ExtractData<PlayerAssign>(packet);

    client_.SetClientId(player_id);

    //std::cout << "[Server][INFO] Assigned Player ID: " << static_cast<int>(player_id) << " with spawn position (" << spawn_x << ", " << spawn_y << ").\n";

    const auto entity = client_.GetGameState().AddPlayer(player_id, spawn_x, spawn_y, score);

    if (entity == client::GameState::InvalidEntity) {
      std::cerr << "[Server][ERROR] Failed to add Player ID: " << static_cast<int>(player_id)
                << " to GameState. Player might already exist.\n";

      client_.Shutdown();
      return;
    }

    client_.GetGameState().SetLocalPlayerEntity(entity);

    //std::cout << "[Server][INFO] Player " << static_cast<int>(player_id) << " successfully added to GameState.\n";

    const auto udp_port = client_.GetNetworkClient().GetLocalUdpPort();
    if (udp_port == 0) {
      std::cerr << "[Client][ERROR] Invalid UDP port. Cannot send to server.\n";
      client_.Shutdown();
      return;
    }

    Packet<MyPacketType> udp_port_packet;
    udp_port_packet.header.type = MyPacketType::kUdpPort;
    udp_port_packet.Push(udp_port);

    client_.GetNetworkClient().SendTcp(std::move(udp_port_packet));
    // std::cout << "[Client][INFO] Sent UDP port: " << udp_port << " to the server.\n";
  }

  void HandleUpdatePlayers(const Packet<MyPacketType>& packet) const {
    const auto update_players =
        PacketFactory<MyPacketType>::ExtractDataArray<UpdatePlayer>(packet);
    auto& game_state = client_.GetGameState();
    auto& registry = game_state.GetRegistry();
    auto& positions = registry.get_components<Position>();
    auto& client_player = registry.get_components<ClientPlayer>();

    for (const auto& [player_id, new_x, new_y, score] : update_players) {
      if (const auto entity = game_state.GetPlayer(player_id);
          entity == client::GameState::InvalidEntity) {
        std::cout << "[Client][INFO] Adding new player with ID: " << static_cast<int>(player_id) << '\n';

        game_state.AddPlayer(player_id, new_x, new_y, score);

        std::cout << "[Client][INFO] Added Player " << static_cast<int>(player_id)
                  << " at position (" << new_x << ", " << new_y << ")\n";
          } else if (entity < positions.size() && positions[entity].has_value() && client_player[entity].has_value()) {
            auto& [x, y] = *positions[entity];
            auto& [_, player_score] = *client_player[entity];
            x = new_x;
            y = new_y;

            player_score = score;

            std::cout << "[Client][INFO] Updated position for Player " << static_cast<int>(player_id)
                      << " to (" << new_x << ", " << new_y << ")\n";
          } else {
            std::cerr << "[Client][WARNING] Position component not found for Player ID: "
                      << static_cast<int>(player_id) << '\n';
          }
    }
  }

  void HandleUpdateEnemies(const Packet<MyPacketType>& packet) const {
    const auto update_enemies =
        PacketFactory<MyPacketType>::ExtractDataArray<UpdateEnemy>(packet);
    auto& game_state = client_.GetGameState();
    auto& registry = game_state.GetRegistry();
    auto& positions = registry.get_components<Position>();

    for (const auto& [enemy_id, new_x, new_y] : update_enemies) {
      if (const auto entity = game_state.GetEnemy(enemy_id);
          entity == client::GameState::InvalidEntity) {
        std::cout << "[Client][INFO] Adding new enemy with ID: " << static_cast<int>(enemy_id) << '\n';

        game_state.AddEnemy(enemy_id, new_x, new_y);

        std::cout << "[Client][INFO] Added Enemy " << static_cast<int>(enemy_id)
                  << " at position (" << new_x << ", " << new_y << ")\n";
          } else if (entity < positions.size() && positions[entity].has_value()) {
            auto& [x, y] = *positions[entity];
            x = new_x;
            y = new_y;

            std::cout << "[Client][INFO] Updated position for Enemy " << static_cast<int>(enemy_id)
                      << " to (" << new_x << ", " << new_y << ")\n";
          } else {
            std::cerr << "[Client][WARNING] Position component not found for Enemy ID: "
                      << static_cast<int>(enemy_id) << '\n';
          }
    }
  }

  void HandleUpdateProjectiles(const Packet<MyPacketType>& packet) const {
    const auto update_projectiles =
        PacketFactory<MyPacketType>::ExtractDataArray<UpdateProjectile>(packet);
    auto& game_state = client_.GetGameState();
    auto& registry = game_state.GetRegistry();
    auto& positions = registry.get_components<Position>();

    for (const auto& [projectile_id, owner_id, new_x, new_y] : update_projectiles) {
      if (const auto entity = game_state.GetProjectileEntity(projectile_id);
          entity == client::GameState::InvalidEntity) {
        std::cout << "[Client][INFO] Adding new projectile with ID: " << projectile_id
                  << " for Owner " << static_cast<int>(owner_id) << " at position ("
                  << new_x << ", " << new_y << ")\n";

        game_state.AddProjectile(projectile_id, owner_id, new_x, new_y);
          } else if (entity < positions.size() && positions[entity].has_value()) {
            auto& [x, y] = *positions[entity];
            x = new_x;
            y = new_y;

            std::cout << "[Client][INFO] Updated position for projectile " << projectile_id
                      << " of Owner " << static_cast<int>(owner_id) << " to ("
                      << new_x << ", " << new_y << ")\n";
          } else {
            std::cerr << "[Client][WARNING] Position component not found for projectile ID: "
                      << projectile_id << '\n';
          }
    }
  }

  static void HandlePong(Packet<MyPacketType>& packet) {
    try {
      if (packet.body.size() == sizeof(std::uint32_t)) {
        const auto timestamp = packet.Extract<std::uint32_t>();

        const auto current_time = std::chrono::steady_clock::now();
        const auto received_time = std::chrono::milliseconds(timestamp);

        const auto ping = std::chrono::duration_cast<std::chrono::milliseconds>(
                              current_time.time_since_epoch())
                              .count() -
                          received_time.count();

        std::cout << "[MessageDispatcher][INFO] Ping received: " << ping << " ms\n";
      } else {
        std::cerr << "[MessageDispatcher][ERROR] Pong packet has insufficient data.\n";
      }
    } catch (const std::exception& e) {
      std::cerr << "[MessageDispatcher][ERROR] Exception while handling Pong: " << e.what() << '\n';
    }
  }

  void HandlePlayerJoin(const Packet<MyPacketType>& packet) const {
    auto [player_id, x, y, score] = PacketFactory<MyPacketType>::ExtractData<PlayerJoin>(packet);

    std::cout << "[Client][INFO] Player " << static_cast<int>(player_id)
              << " joined the game at position (" << x << ", " << y << ").\n";

    client_.GetGameState().AddPlayer(player_id, x, y, score);

    std::cout << "[Client][INFO] Player " << static_cast<int>(player_id)
              << " successfully added to local GameState.\n";
  }

  void HandlePlayerLeave(const Packet<MyPacketType>& packet) const {
    auto [player_id] = PacketFactory<MyPacketType>::ExtractData<PlayerLeave>(packet);

    std::cout << "[Client][INFO] Player " << static_cast<int>(player_id) << " has left the game.\n";

    client_.GetGameState().RemovePlayer(player_id);

    std::cout << "[Client][INFO] Player " << static_cast<int>(player_id)
              << " successfully removed locally.\n";
  }

  void HandleRemoveProjectile(const Packet<MyPacketType>& packet) const {
    const auto [projectile_id] = PacketFactory<MyPacketType>::ExtractData<RemoveProjectile>(packet);

    client_.GetGameState().RemoveProjectile(projectile_id);
  }

  void HandleRemoveEnemy(const Packet<MyPacketType>& packet) const {
    const auto [enemy_id] = PacketFactory<MyPacketType>::ExtractData<RemoveEnemy>(packet);

    client_.GetGameState().RemoveEnemy(enemy_id);
  }

  Client& client_;
  std::array<Handler, static_cast<size_t>(MyPacketType::kMaxTypes)> handlers_;
};

} // namespace network

#endif  // MESSAGE_DISPATCHER_HPP_
