#ifndef JOIN_LOBBY_HANDLER_HPP_
#define JOIN_LOBBY_HANDLER_HPP_

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
void HandleJoinLobby(const std::shared_ptr<void>& raw_event,
                     ServiceContainer& service_container,
                     GameState<PacketType>& game_state) {
    auto event = std::static_pointer_cast<
        std::pair<network::Packet<PacketType>,
                  std::shared_ptr<network::TcpServerConnection<PacketType>>>>(raw_event);

    const auto& packet = event->first;
    const auto& connection = event->second;

    // Vérifier la taille du paquet
    if (packet.body.size() != sizeof(network::packets::JoinLobbyPacket)) {
        std::cerr << "[JoinLobbyHandler][ERROR] Invalid JoinLobbyPacket size." << std::endl;
        auto response_packet = network::CreateJoinLobbyResponsePacket<PacketType>(400); // Bad Request
        connection->Send(response_packet);
        return;
    }

    const int connection_id = connection->GetId();
    const auto player_db_id_opt = game_state.GetDbIdByConnectionId(connection_id);

    if (!player_db_id_opt) {
        std::cerr << "[JoinLobbyHandler][ERROR] Player is not connected: Connection ID "
                  << connection_id << std::endl;
        auto response_packet = network::CreateJoinLobbyResponsePacket<PacketType>(401); // Unauthorized
        connection->Send(response_packet);
        return;
    }

    const int player_db_id = *player_db_id_opt;
    const auto* join_request =
        reinterpret_cast<const network::packets::JoinLobbyPacket*>(packet.body.data());
    const int lobby_id = join_request->lobby_id;
    const std::string provided_password(join_request->password);

    const auto lobby_service = service_container.GetLobbyService();
    const auto lobby_player_service = service_container.GetLobbyPlayerService();
    const auto user_service = service_container.GetUserService();

    if (!lobby_service || !lobby_player_service || !user_service) {
        std::cerr << "[JoinLobbyHandler][ERROR] Required services are not available." << std::endl;
        auto response_packet = network::CreateJoinLobbyResponsePacket<PacketType>(500); // Internal Server Error
        connection->Send(response_packet);
        return;
    }

    // Récupérer le lobby et valider l'accès
    const auto lobby_opt = lobby_service->GetLobbyById(lobby_id);
    if (!lobby_opt) {
        std::cerr << "[JoinLobbyHandler][ERROR] Lobby not found: " << lobby_id << "." << std::endl;
        auto response_packet = network::CreateJoinLobbyResponsePacket<PacketType>(404); // Lobby Not Found
        connection->Send(response_packet);
        return;
    }

    const auto& lobby = *lobby_opt;
    if (!lobby_service->CanJoinLobby(lobby, provided_password)) {
        std::cerr << "[JoinLobbyHandler][ERROR] Access denied for lobby ID: " << lobby_id << "." << std::endl;
        auto response_packet = network::CreateJoinLobbyResponsePacket<PacketType>(403); // Forbidden
        connection->Send(response_packet);
        return;
    }

    // Ajouter le joueur au lobby
    if (!lobby_player_service->AddPlayerToLobby(player_db_id, lobby.id)) {
        std::cerr << "[JoinLobbyHandler][ERROR] Failed to add player " << player_db_id
                  << " to lobby " << lobby.id << "." << std::endl;
        auto response_packet = network::CreateJoinLobbyResponsePacket<PacketType>(500); // Internal Server Error
        connection->Send(response_packet);
        return;
    }

    // Récupérer le profil utilisateur
    auto user_profile_opt = user_service->GetUserProfile(player_db_id);
    if (!user_profile_opt) {
        std::cerr << "[JoinLobbyHandler][ERROR] User profile not found for player ID " << player_db_id << "." << std::endl;
        auto response_packet = network::CreateJoinLobbyResponsePacket<PacketType>(500); // Internal Server Error
        connection->Send(response_packet);
        return;
    }

    const auto& user_profile = *user_profile_opt;

    // Créer une seule fois le paquet de notification
    auto notification_packet = network::CreatePlayerJoinedLobbyPacket<PacketType>(
        player_db_id, user_profile.username);

    // Notification des autres joueurs
    const auto players_in_lobby = lobby_player_service->GetPlayersInLobby(lobby.id);
    for (const int other_player_id : players_in_lobby) {
        if (other_player_id != player_db_id) {
            if (const auto other_connection = game_state.GetPlayerConnectionByDbId(other_player_id)) {
                other_connection->Send(notification_packet); // Envoie le même paquet
            }
        }
    }

    // Envoyer la réponse de succès
    auto response_packet = network::CreateJoinLobbyResponsePacket<PacketType>(200); // Success
    connection->Send(response_packet);

    std::cout << "[JoinLobbyHandler] Player " << user_profile.username
              << " (" << player_db_id << ") joined lobby "
              << lobby.name << " (ID: " << lobby.id << ")." << std::endl;
}

}  // namespace rtype

#endif  // JOIN_LOBBY_HANDLER_HPP_
