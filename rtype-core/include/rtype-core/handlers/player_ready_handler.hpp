#ifndef HANDLE_PLAYER_READY_HPP_
#define HANDLE_PLAYER_READY_HPP_

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

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

  // Vérification de la taille du paquet
  if (packet.body.size() != sizeof(network::packets::PlayerReadyPacket)) {
    std::cerr << "[PlayerReadyHandler][ERROR] Invalid PlayerReadyPacket size."
              << std::endl;
    auto response_packet = network::CreatePlayerReadyPacketResponse<PacketType>(
        400);  // Code 400: Bad Request
    connection->Send(response_packet);
    return;
  }

  const auto* ready_data =
      reinterpret_cast<const network::packets::PlayerReadyPacket*>(
          packet.body.data());
  const bool is_ready = ready_data->is_ready;

  const int connection_id = connection->GetId();
  const auto player_db_id_opt = game_state.GetDbIdByConnectionId(connection_id);

  // Vérifiez que le joueur est connecté
  if (!player_db_id_opt) {
    std::cerr
        << "[PlayerReadyHandler][ERROR] Player is not connected: Connection ID "
        << connection_id << std::endl;
    auto response_packet = network::CreatePlayerReadyPacketResponse<PacketType>(
        401);  // Code 401: Unauthorized
    connection->Send(response_packet);
    return;
  }

  const int player_id = *player_db_id_opt;

  // Récupérer le service de gestion des joueurs dans les lobbies
  const auto lobby_player_service = service_container.GetLobbyPlayerService();
  if (!lobby_player_service) {
    std::cerr << "[PlayerReadyHandler][ERROR] LobbyPlayerService not available."
              << std::endl;
    auto response_packet = network::CreatePlayerReadyPacketResponse<PacketType>(
        500);  // Code 500: Internal Server Error
    connection->Send(response_packet);
    return;
  }

  const auto lobby_id_opt = lobby_player_service->GetLobbyForPlayer(player_id);

  // Vérifiez que le joueur appartient à un lobby
  if (!lobby_id_opt) {
    std::cerr << "[PlayerReadyHandler][ERROR] Lobby not found for player "
              << player_id << std::endl;
    auto response_packet = network::CreatePlayerReadyPacketResponse<PacketType>(
        404);  // Code 404: Not Found
    connection->Send(response_packet);
    return;
  }

  const int lobby_id = *lobby_id_opt;

  // Récupérer le service de gestion des lobbies
  const auto lobby_service = service_container.GetLobbyService();
  if (!lobby_service) {
    std::cerr << "[PlayerReadyHandler][ERROR] LobbyService not available."
              << std::endl;
    auto response_packet = network::CreatePlayerReadyPacketResponse<PacketType>(
        500);  // Code 500: Internal Server Error
    connection->Send(response_packet);
    return;
  }

  // Vérifiez si une partie est déjà en cours
  if (lobby_service->IsGameActive(lobby_id)) {
    std::cerr << "[PlayerReadyHandler][ERROR] Game is already active for lobby "
              << lobby_id << std::endl;
    auto response_packet = network::CreatePlayerReadyPacketResponse<PacketType>(
        403);  // Code 403: Forbidden
    connection->Send(response_packet);
    return;
  }

  // Mettre à jour le statut de readiness du joueur
  if (!lobby_player_service->SetPlayerReadyStatus(player_id, is_ready)) {
    std::cerr
        << "[PlayerReadyHandler][ERROR] Failed to set ready status for player "
        << player_id << std::endl;
    auto response_packet = network::CreatePlayerReadyPacketResponse<PacketType>(
        500);  // Code 500: Internal Server Error
    connection->Send(response_packet);
    return;
  }

  // Envoyer une notification à tous les autres joueurs du lobby
  auto notification_packet =
      network::CreateLobbyPlayerReadyPacket<PacketType>(player_id, is_ready);
  const auto players_in_lobby =
      lobby_player_service->GetPlayersInLobby(lobby_id);

  for (const int other_player_id : players_in_lobby) {
    if (const auto other_connection =
            game_state.GetPlayerConnectionByDbId(other_player_id)) {
      other_connection->Send(notification_packet);
    }
  }

  // Vérifier si tous les joueurs sont prêts
  if (lobby_player_service->AreAllPlayersReady(lobby_id)) {
    std::cout << "[PlayerReadyHandler] All players in lobby " << lobby_id
              << " are ready. Updating lobby status..." << std::endl;

    // Mettre à jour le statut du lobby
    if (!lobby_service->StartGame(lobby_id)) {
      std::cerr << "[PlayerReadyHandler][ERROR] Failed to mark game as in "
                   "progress for lobby "
                << lobby_id << std::endl;
      auto response_packet =
          network::CreatePlayerReadyPacketResponse<PacketType>(
              500);  // Code 500: Internal Server Error
      connection->Send(response_packet);
      return;
    }

    std::cout << "[PlayerReadyHandler] Lobby " << lobby_id
              << " is now marked as in progress." << std::endl;

    // TODO: Ajouter ici la logique pour démarrer le Pod Kubernetes si
    // nécessaire

    std::string token;
    try {
      std::ifstream tokenFile("/var/run/secrets/kubernetes.io/serviceaccount/token");
      if (!tokenFile.is_open()) {
        throw std::ios_base::failure("Failed to open token file");
      }

      // Lire le contenu du fichier
      token.assign(std::istreambuf_iterator<char>(tokenFile), std::istreambuf_iterator<char>());
      tokenFile.close();

      if (token.empty()) {
        throw std::runtime_error("Token file is empty");
      }
    } catch (const std::exception& e) {
      std::cerr << "[PlayerReadyHandler][ERROR] Exception while reading token: "
                << e.what() << std::endl;
      auto response_packet = network::CreatePlayerReadyPacketResponse<PacketType>(500);
      connection->Send(response_packet);
      return;
    }

    const std::string kubeApiUrl = "https://kubernetes.default.svc";
    const std::string podName = "game-pod-" + std::to_string(lobby_id);

    std::cout << "[PlayerReadyHandler] Starting game Pod for lobby " << lobby_id
              << "..." << std::endl;
    std::cout << "[PlayerReadyHandler] Pod Name: " << podName << std::endl;
    std::cout << "[PlayerReadyHandler] Kubernetes API URL: " << kubeApiUrl
              << std::endl;

    auto pod_endpoint_opt =
        ServiceContainer::CreatePodAndService(podName, kubeApiUrl, token);
    if (pod_endpoint_opt) {
      const auto& [ip, ports] = *pod_endpoint_opt;

      std::cout << "[PlayerReadyHandler] Game Pod started." << std::endl;
      std::cout << "[PlayerReadyHandler] IP: " << ip << std::endl;
      std::cout << "[PlayerReadyHandler] Ports: ";
      for (const auto& port : ports) {
        std::cout << port << " ";
      }
      std::cout << std::endl;

      // Créer le packet de connexion au jeu
      auto game_start_packet = network::CreateGameConnectionInfoPacket<PacketType>(ip, ports);

      for (const int player_id_in_lobby : players_in_lobby) {
        if (const auto player_connection = game_state.GetPlayerConnectionByDbId(player_id_in_lobby)) {
          player_connection->Send(game_start_packet);

          std::cout << "[PlayerReadyHandler] Sent GameConnectionInfoPacket to player ID: "
                    << player_id_in_lobby << std::endl;
        } else {
          std::cerr << "[PlayerReadyHandler] Failed to get connection for player ID: "
                    << player_id_in_lobby << std::endl;
        }
      }
    } else {
      std::cerr
          << "[PlayerReadyHandler][ERROR] Failed to start game Pod for lobby "
          << lobby_id << std::endl;
      auto response_packet =
          network::CreatePlayerReadyPacketResponse<PacketType>(
              500);  // Code 500: Internal Server Error
      connection->Send(response_packet);
      return;
    }
  }

  // Envoyer une réponse de succès au joueur actuel
  auto response_packet = network::CreatePlayerReadyPacketResponse<PacketType>(
      200);  // Code 200: Success
  connection->Send(response_packet);

  std::cout << "[PlayerReadyHandler] Player " << player_id
            << " updated readiness to " << (is_ready ? "Ready" : "Not Ready")
            << " in lobby " << lobby_id << "." << std::endl;
}

}  // namespace rtype

#endif  // HANDLE_PLAYER_READY_HPP_
