#ifndef LEAVE_LOBBY_HANDLER_HPP_
#define LEAVE_LOBBY_HANDLER_HPP_

#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "rtype-core/service_container.hpp"
#include "rtype-core/game_state.hpp"
#include "rtype-core/protocol.hpp"

namespace rtype {

template <typename PacketType>
void HandleLeaveLobby(const std::shared_ptr<void>& raw_event,
                      ServiceContainer& service_container,
                      GameState<PacketType>& game_state) {
    auto event = std::static_pointer_cast<
        std::pair<network::Packet<PacketType>,
                  std::shared_ptr<network::TcpServerConnection<PacketType>>>>(raw_event);

    const auto& packet = event->first;
    const auto& connection = event->second;

    const int connection_id = connection->GetId();
    const auto player_db_id_opt = game_state.GetDbIdByConnectionId(connection_id);

    if (!player_db_id_opt) {
        std::cerr << "[LeaveLobbyHandler][ERROR] Player is not connected: Connection ID " << connection_id << std::endl;
        auto response_packet = network::CreateLeaveLobbyResponsePacket<PacketType>(401);  // Unauthorized
        connection->Send(std::move(response_packet));
        return;
    }

    const int player_db_id = *player_db_id_opt;

    const auto lobby_player_service = service_container.GetLobbyPlayerService();
    if (!lobby_player_service) {
        std::cerr << "[LeaveLobbyHandler][ERROR] LobbyPlayerService not available." << std::endl;
        auto response_packet = network::CreateLeaveLobbyResponsePacket<PacketType>(500);  // Internal server error
        connection->Send(std::move(response_packet));
        return;
    }

    // Vérifier si le joueur est dans un lobby
    const auto lobby_id_opt = lobby_player_service->GetLobbyForPlayer(player_db_id);
    if (!lobby_id_opt) {
        std::cerr << "[LeaveLobbyHandler][ERROR] Player " << player_db_id << " is not in any lobby." << std::endl;
        auto response_packet = network::CreateLeaveLobbyResponsePacket<PacketType>(404);  // Lobby not found
        connection->Send(std::move(response_packet));
        return;
    }

    // Retirer le joueur du lobby
    if (!lobby_player_service->RemovePlayerFromLobby(player_db_id)) {
        std::cerr << "[LeaveLobbyHandler][ERROR] Failed to remove player " << player_db_id << " from the lobby." << std::endl;
        auto response_packet = network::CreateLeaveLobbyResponsePacket<PacketType>(500);  // Internal server error
        connection->Send(std::move(response_packet));
        return;
    }


  auto notification_packet = network::CreatePlayerLeftLobbyNotificationPacket<PacketType>(player_db_id);

  for (const auto players_in_lobby = lobby_player_service->GetPlayersInLobby(*lobby_id_opt);
       const int other_player_id : players_in_lobby) {
    if (other_player_id == player_db_id) {
      continue;
    }

    if (const auto other_connection = game_state.GetPlayerConnectionByDbId(other_player_id)) {
      other_connection->Send(notification_packet);
    }
  }


  auto response_packet = network::CreateLeaveLobbyResponsePacket<PacketType>(200);  // Success
    connection->Send(std::move(response_packet));

    std::cout << "[LeaveLobbyHandler] Player " << player_db_id << " successfully left the lobby." << std::endl;
}

}  // namespace rtype

#endif  // LEAVE_LOBBY_HANDLER_HPP_
