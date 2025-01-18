#ifndef GET_LOBBY_PLAYERS_HANDLER_HPP_
#define GET_LOBBY_PLAYERS_HANDLER_HPP_

#include <iostream>
#include <memory>
#include <vector>
#include <cstring>
#include "rtype-core/service_container.hpp"
#include "rtype-core/game_state.hpp"
#include "rtype-core/protocol.hpp"

namespace rtype {

template <typename PacketType>
void HandleGetLobbyPlayers(const std::shared_ptr<void>& raw_event,
                           ServiceContainer& service_container,
                           GameState<PacketType>& game_state) {
    auto event = std::static_pointer_cast<
        std::pair<network::Packet<PacketType>,
                  std::shared_ptr<network::TcpServerConnection<PacketType>>>>(raw_event);

    const auto& packet = event->first;
    const auto& connection = event->second;

    if (packet.body.size() != sizeof(network::packets::GetLobbyPlayersPacket)) {
        std::cerr << "[GetLobbyPlayersHandler][ERROR] Invalid GetLobbyPlayersPacket size." << std::endl;
        return;
    }

    const auto* request = reinterpret_cast<const network::packets::GetLobbyPlayersPacket*>(packet.body.data());
    const int lobby_id = request->lobby_id;

    const auto connection_id = connection->GetId();
    const auto player_db_id_opt = game_state.GetDbIdByConnectionId(connection_id);

    if (!player_db_id_opt) {
        std::cerr << "[GetLobbyPlayersHandler][ERROR] Player is not connected: Connection ID " << connection_id << std::endl;
        auto response_packet = network::CreateGetLobbyPlayersResponsePacket<PacketType>(
            401, {});  // Code 401: Unauthorized
        connection->Send(std::move(response_packet));
        return;
    }

    const auto lobby_service = service_container.GetLobbyPlayerService();
    if (!lobby_service) {
        std::cerr << "[GetLobbyPlayersHandler][ERROR] LobbyPlayerService not available." << std::endl;
        auto response_packet = network::CreateGetLobbyPlayersResponsePacket<PacketType>(
            500, {});  // Code 500: Internal server error
        connection->Send(std::move(response_packet));
        return;
    }

    const auto user_service = service_container.GetUserService();
    if (!user_service) {
        std::cerr << "[GetLobbyPlayersHandler][ERROR] UserService not available." << std::endl;
        auto response_packet = network::CreateGetLobbyPlayersResponsePacket<PacketType>(
            500, {});  // Code 500: Internal server error
        connection->Send(std::move(response_packet));
        return;
    }

    // Récupération des joueurs et de leurs statuts
    const auto players_with_status = lobby_service->GetPlayersWithStatusInLobby(lobby_id);
    if (players_with_status.empty()) {
        std::cerr << "[GetLobbyPlayersHandler][INFO] No players found for lobby ID: " << lobby_id << "." << std::endl;
        auto response_packet = network::CreateGetLobbyPlayersResponsePacket<PacketType>(
            404, {});  // Code 404: Lobby not found
        connection->Send(std::move(response_packet));
        return;
    }

    // Construction de la réponse
    std::vector<network::packets::GetLobbyPlayersResponsePacket::PlayerInfo> player_info_list;
    for (const auto& [player_id, is_ready] : players_with_status) {
        auto user_profile_opt = user_service->GetUserProfile(player_id);
        if (!user_profile_opt) {
            std::cerr << "[GetLobbyPlayersHandler][WARNING] No user profile found for player ID " << player_id << "." << std::endl;
            continue;
        }

        const auto& user_profile = *user_profile_opt;
        network::packets::GetLobbyPlayersResponsePacket::PlayerInfo player_info{};
        player_info.player_id = player_id;
        player_info.is_ready = is_ready;
        std::strncpy(player_info.username, user_profile.username.c_str(), sizeof(player_info.username) - 1);
        player_info.username[sizeof(player_info.username) - 1] = '\0'; // Assurez-vous que c'est nul-terminé
        player_info_list.push_back(player_info);
    }

    // Envoi de la réponse
    auto response_packet = network::CreateGetLobbyPlayersResponsePacket<PacketType>(
        200, player_info_list);  // Code 200: Success
    connection->Send(std::move(response_packet));

    std::cout << "[GetLobbyPlayersHandler] Sent lobby player list for lobby ID " << lobby_id << "." << std::endl;
}

}  // namespace rtype

#endif  // GET_LOBBY_PLAYERS_HANDLER_HPP_
