#ifndef MESSAGE_DISPATCHER_H_
#define MESSAGE_DISPATCHER_H_

#include <array>
#include <functional>
#include <network/owned_packet.hpp>
#include <network/protocol.hpp>
#include <shared/my_packet_types.hpp>
#include <shared/network_messages.hpp>
#include <shared/player_actions.hpp>

#include "custom_network_server.hpp"

namespace network {

class MessageDispatcher {
 public:
  using TCPHandler = std::function<void(Packet<MyPacketType>&&, const std::shared_ptr<TcpServerConnection<MyPacketType>>&)>;
  using UDPHandler = std::function<void(Packet<MyPacketType>&&, const asio::ip::udp::endpoint&)>;

  explicit MessageDispatcher(CustomNetworkServer<MyPacketType>& server)
      : server_(server) {
    InitializeHandlers();
  }

  void dispatch(OwnedPacket<MyPacketType>&& owned_packet) {
    std::visit(
        [this]<typename T0>(T0&& packet_variant) {
            this->handle_packet(std::forward<T0>(packet_variant));
        },
        std::move(owned_packet)
    );
  }

 private:
  void handle_packet(OwnedPacketTCP<MyPacketType>&& packet_variant) const {
    auto& packet = packet_variant.packet;
    const auto& connection = packet_variant.connection;

    if (const auto index = static_cast<size_t>(packet.header.type);
        index < tcp_handlers_.size() && tcp_handlers_[index]) {
      tcp_handlers_[index](std::move(packet), connection);
    } else {
      DefaultTCPHandler(std::move(packet), connection);
    }
  }

  void handle_packet(OwnedPacketUDP<MyPacketType>&& packet_variant) const {
    auto& packet = packet_variant.packet;
    const auto& endpoint = packet_variant.endpoint;

    if (const auto index = static_cast<size_t>(packet.header.type);
        index < udp_handlers_.size() && udp_handlers_[index]) {
      udp_handlers_[index](std::move(packet), endpoint);
    } else {
      DefaultUDPHandler(std::move(packet), endpoint);
    }
  }

  static void DefaultTCPHandler(Packet<MyPacketType>&& packet, const std::shared_ptr<TcpServerConnection<MyPacketType>>& /*connection*/) {
    std::cerr << "[MessageDispatcher][TCP] Unhandled packet "
              << packet << std::endl;
  }

  static void DefaultUDPHandler(Packet<MyPacketType>&& packet, const asio::ip::udp::endpoint& /*endpoint*/) {
    std::cerr << "[MessageDispatcher][UDP] Unhandled packet "
              << packet << std::endl;
  }

  static void HandlePingTCP(Packet<MyPacketType>&& packet, const std::shared_ptr<TcpServerConnection<MyPacketType>>& connection) {
    try {
      if (packet.body.size() == sizeof(std::uint32_t)) {
        packet.header.type = MyPacketType::kPong;
        connection->Send(packet);
      } else {
        std::cerr << "[MessageDispatcher] Ping packet has insufficient data.\n";
      }
    } catch (const std::exception& e) {
      std::cerr << "[MessageDispatcher][ERROR] " << e.what() << "\n";
    }
  }

  void HandlePingUdp(Packet<MyPacketType>&& packet, const asio::ip::udp::endpoint& endpoint) const {
    try {
      if (packet.body.size() == sizeof(std::uint32_t)) {
        packet.header.type = MyPacketType::kPong;
        server_.SendToUdp(endpoint, std::move(packet));
        // std::cout << "[MessageDispatcher][UDP] Ping received from: " << endpoint << "\n";
      } else {
        std::cerr << "[MessageDispatcher][UDP] Ping packet has insufficient data.\n";
      }
    } catch (const std::exception& e) {
      std::cerr << "[MessageDispatcher][UDP][ERROR] " << e.what() << "\n";
    }
  }

  void HandlePlayerInputUDP(Packet<MyPacketType>&& packet, const asio::ip::udp::endpoint& /*endpoint*/) const {
    try {
      if (packet.body.size() == sizeof(PlayerInput)) {
        auto& game_state = server_.GetGameState();

        auto [player_id, input_actions, norm_mouse_x, norm_mouse_y, timestamp] =
            PacketFactory<MyPacketType>::ExtractData<PlayerInput>(packet);

        const auto entity = game_state.GetEntityByPlayerId(player_id);
        if (entity == static_cast<Registry::entity_t>(-1)) {
          std::cerr << "[MessageDispatcher] Player entity not found for player_id: "
                    << static_cast<int>(player_id) << "\n";
          return;
        }

        auto& registry = game_state.get_registry();
        auto& actions = registry.get_components<Actions>();
        auto& positions = registry.get_components<Position>();
        auto& last_shot_times = registry.get_components<LastShotTime>();

        if (entity < actions.size() && actions[entity].has_value()) {
          actions[entity]->current_actions = input_actions;

          if (input_actions & static_cast<uint16_t>(PlayerAction::Shoot)) {
            if (entity < last_shot_times.size() && last_shot_times[entity].has_value() &&
                entity < positions.size() && positions[entity].has_value()) {
              auto& last_shot_time = last_shot_times[entity]->last_shot_time;

              if (const auto current_time = std::chrono::milliseconds(timestamp);
                  current_time - last_shot_time >= std::chrono::milliseconds(200)) {
                last_shot_time = current_time;

                const auto player_x = positions[entity]->x;
                const auto player_y = positions[entity]->y;

                const float dir_x = norm_mouse_x;
                const float dir_y = norm_mouse_y;

                const float length = std::sqrt(dir_x * dir_x + dir_y * dir_y);
                if (length > 0.01f) {
                  const float norm_x = dir_x / length;
                  const float norm_y = dir_y / length;

                  game_state.AddProjectile(player_id, player_x, player_y, norm_x, norm_y);

                  std::cout << "[MessageDispatcher] Player " << static_cast<int>(player_id)
                            << " fired a projectile with direction (" << norm_x << ", " << norm_y << ").\n";
                } else {
                  std::cerr << "[MessageDispatcher][WARNING] Invalid direction vector from player "
                            << static_cast<int>(player_id) << ".\n";
                }
              }
            }
          }
        }
      } else {
        std::cerr << "[MessageDispatcher] PlayerInput packet has insufficient data.\n";
      }
    } catch (const std::exception& e) {
      std::cerr << "[MessageDispatcher][ERROR] Failed to process PlayerInput: " << e.what() << "\n";
    }
  }

  void HandleUdpPort(Packet<MyPacketType>&& packet, const std::shared_ptr<TcpServerConnection<MyPacketType>>& connection) const {
    try {
      if (packet.body.size() == sizeof(uint16_t)) {
        const auto udp_port = PacketFactory<MyPacketType>::ExtractData<uint16_t>(packet);

        server_.RegisterUdpEndpoint(connection, udp_port);

        std::cout << "[MessageDispatcher][INFO] Registered UDP port " << udp_port
                  << " for client ID " << connection->GetId() << std::endl;
      } else {
        std::cerr << "[MessageDispatcher][ERROR] Invalid UDP port packet size from client ID "
                  << connection->GetId() << std::endl;
      }
    } catch (const std::exception& e) {
      std::cerr << "[MessageDispatcher][ERROR] Failed to process UDP port packet: " << e.what() << "\n";
    }
  }

  void InitializeHandlers() {
    tcp_handlers_.fill(nullptr);
    udp_handlers_.fill(nullptr);

    tcp_handlers_[static_cast<size_t>(MyPacketType::kPing)] = [this](Packet<MyPacketType>&& packet, const std::shared_ptr<TcpServerConnection<MyPacketType>>& connection) {
      HandlePingTCP(std::move(packet), connection);
    };

    tcp_handlers_[static_cast<size_t>(MyPacketType::kUdpPort)] = [this](Packet<MyPacketType>&& packet, const std::shared_ptr<TcpServerConnection<MyPacketType>>& connection) {
      HandleUdpPort(std::move(packet), connection);
    };

    udp_handlers_[static_cast<size_t>(MyPacketType::kPing)] = [this](Packet<MyPacketType>&& packet, const asio::ip::udp::endpoint& endpoint) {
      HandlePingUdp(std::move(packet), endpoint);
    };

    udp_handlers_[static_cast<size_t>(MyPacketType::kPlayerInput)] = [this](Packet<MyPacketType>&& packet, const asio::ip::udp::endpoint& endpoint) {
      HandlePlayerInputUDP(std::move(packet), endpoint);
    };
  }

  CustomNetworkServer<MyPacketType>& server_;

  std::array<TCPHandler, static_cast<size_t>(MyPacketType::kMaxTypes)> tcp_handlers_;
  std::array<UDPHandler, static_cast<size_t>(MyPacketType::kMaxTypes)> udp_handlers_;
};

}  // namespace network

#endif  // MESSAGE_DISPATCHER_H_
