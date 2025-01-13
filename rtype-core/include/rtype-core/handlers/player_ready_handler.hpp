#ifndef HANDLE_PLAYER_READY_HPP_
#define HANDLE_PLAYER_READY_HPP_

#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "rtype-core/game_state.hpp"
#include "rtype-core/protocol.hpp"
#include "rtype-core/service_container.hpp"

namespace rtype {
template <typename PacketType>
void HandlePlayerReady(const std::shared_ptr<void>& raw_event,
                       ServiceContainer& service_container,
                       GameState<PacketType>& game_state) {
  auto event = std::static_pointer_cast<
      std::pair<network::Packet<PacketType>,
                std::shared_ptr<network::TcpServerConnection<PacketType>>>>(
      raw_event);

  const auto& packet = event->first;
  const auto& connection = event->second;

  if (packet.body.size() != sizeof(network::packets::PlayerReadyPacket)) {
    std::cerr << "[PlayerReadyHandler][ERROR] Invalid PlayerReadyPacket size."
              << std::endl;
    auto response_packet = network::CreatePlayerReadyPacketResponse<PacketType>(
        400);  // Code 400: Bad request
    connection->Send(std::move(response_packet));
    return;
  }

  const auto* ready_data =
      reinterpret_cast<const network::packets::PlayerReadyPacket*>(
          packet.body.data());
  const bool is_ready = ready_data->is_ready;
  const int player_id = connection->GetId();

  if (!game_state.IsPlayerActive(player_id)) {
    std::cerr << "[PlayerReadyHandler][ERROR] Player is not connected: ID "
              << player_id << std::endl;
    auto response_packet = network::CreatePlayerReadyPacketResponse<PacketType>(
        401);  // Code 401: Unauthorized
    connection->Send(std::move(response_packet));
    return;
  }

  std::cout << "[PlayerReadyHandler] Player " << player_id << " is "
            << (is_ready ? "ready" : "not ready") << ".\n";

  if (const auto lobby_player_service =
          service_container.GetLobbyPlayerService()) {
    if (auto lobby_id_opt =
            lobby_player_service->GetLobbyForPlayer(player_id)) {
      const int lobby_id = *lobby_id_opt;
      if (lobby_player_service->SetPlayerReadyStatus(player_id, is_ready)) {
        if (lobby_player_service->AreAllPlayersReady(lobby_id)) {
          std::cout << "[PlayerReadyHandler] All players in lobby " << lobby_id
                    << " are ready. Game can start!\n";

          // Create a Kubernetes Pod for the lobby
          const std::string podName = "lobby-" + std::to_string(lobby_id);
          const std::string kubeApiUrl = "https://kubernetes.default.svc";
          std::string token;

          if (std::ifstream tokenFile(
                  "/var/run/secrets/kubernetes.io/serviceaccount/token");
              tokenFile.is_open()) {
            token.assign((std::istreambuf_iterator<char>(tokenFile)),
                         std::istreambuf_iterator<char>());
            tokenFile.close();
          } else {
            std::cerr << "[PlayerReadyHandler][ERROR] Failed to read Kubernetes token." << std::endl;
            auto response_packet = network::CreatePlayerReadyPacketResponse<PacketType>(
                500);  // Code 500: Server error
            connection->Send(std::move(response_packet));
            return;
          }

          if (ServiceContainer::CreatePod(podName, kubeApiUrl, token)) {
            std::cout << "[PlayerReadyHandler] Pod for lobby " << lobby_id
                                  << " created successfully." << std::endl;
            auto response_packet = network::CreatePlayerReadyPacketResponse<PacketType>(
                200);  // Code 200: Success
            connection->Send(std::move(response_packet));
          } else {
            std::cerr << "[PlayerReadyHandler][ERROR] Failed to create Pod for lobby "
                                 << lobby_id << std::endl;
            auto response_packet = network::CreatePlayerReadyPacketResponse<PacketType>(
                500);  // Code 500: Server error
            connection->Send(std::move(response_packet));
          }
        } else {
          std::cout << "[PlayerReadyHandler] Player " << player_id
                    << " is ready. Waiting for other players..." << std::endl;
          auto response_packet = network::CreatePlayerReadyPacketResponse<PacketType>(
                        202);  // Code 202: Accepted
          connection->Send(std::move(response_packet));
        }
      } else {
        std::cerr << "[PlayerReadyHandler][ERROR] Failed to set ready status for player "
                          << player_id << std::endl;
        auto response_packet = network::CreatePlayerReadyPacketResponse<PacketType>(
            500);  // Code 500: Server error
        connection->Send(std::move(response_packet));
      }
    } else {
      std::cerr << "[PlayerReadyHandler][ERROR] Lobby not found for player " << player_id << std::endl;
      auto response_packet = network::CreatePlayerReadyPacketResponse<PacketType>(
          404);  // Code 404: Not found
      connection->Send(std::move(response_packet));
    }
  } else {
    std::cerr << "[PlayerReadyHandler][ERROR] LobbyPlayerService not available." << std::endl;
    auto response_packet = network::CreatePlayerReadyPacketResponse<PacketType>(
        500);  // Code 500: Server error
    connection->Send(std::move(response_packet));
  }
}

}  // namespace rtype

#endif  // HANDLE_PLAYER_READY_HPP_
