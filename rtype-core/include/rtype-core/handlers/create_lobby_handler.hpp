#ifndef CREATE_LOBBY_HANDLER_HPP_
#define CREATE_LOBBY_HANDLER_HPP_

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
void HandleCreateLobby(const std::shared_ptr<void>& raw_event,
                       ServiceContainer& service_container,
                       GameState<PacketType>& game_state) {
    auto event = std::static_pointer_cast<
        std::pair<network::Packet<PacketType>,
                  std::shared_ptr<network::TcpServerConnection<PacketType>>>>(raw_event);

    const auto& packet = event->first;
    const auto& connection = event->second;

    if (packet.body.size() != sizeof(network::packets::CreateLobbyPacket)) {
        std::cerr << "[CreateLobbyHandler][ERROR] Invalid CreateLobbyPacket size." << std::endl;
        auto error_response = network::CreateCreateLobbyResponsePacket<PacketType>(
            400);  // Code 400: Invalid request
        connection->Send(std::move(error_response));
        return;
    }

    const int connection_id = connection->GetId();
    const auto creator_db_id_opt = game_state.GetDbIdByConnectionId(connection_id);

    if (!creator_db_id_opt) {
        std::cerr << "[CreateLobbyHandler][ERROR] Creator is not connected: Connection ID "
                  << connection_id << std::endl;
        auto error_response = network::CreateCreateLobbyResponsePacket<PacketType>(
            401);  // Code 401: Unauthorized
        connection->Send(std::move(error_response));
        return;
    }

    const int creator_db_id = *creator_db_id_opt;
    const auto* lobby_data =
        reinterpret_cast<const network::packets::CreateLobbyPacket*>(packet.body.data());
    const std::string lobby_name(lobby_data->name);
    const std::string password(lobby_data->password);

    if (const auto lobby_service = service_container.GetLobbyService()) {
        // Création du lobby
        auto lobby_result = lobby_service->CreateLobby(
            lobby_name, password.empty() ? std::nullopt : std::optional{password});

        if (!lobby_result.has_value()) {
            std::cerr << "[CreateLobbyHandler][ERROR] Failed to create lobby: "
                      << lobby_name << std::endl;
            auto error_response = network::CreateCreateLobbyResponsePacket<PacketType>(
                500);  // Code 500: Server error
            connection->Send(std::move(error_response));
            return;
        }

        const auto& lobby = *lobby_result;

        // Ajout du créateur au lobby
        if (const auto lobby_player_service = service_container.GetLobbyPlayerService()) {
            if (!lobby_player_service->AddPlayerToLobby(creator_db_id, lobby.id)) {
                std::cerr << "[CreateLobbyHandler][ERROR] Failed to add player " << creator_db_id
                          << " to lobby " << lobby.id << std::endl;
                auto error_response = network::CreateCreateLobbyResponsePacket<PacketType>(
                    500);  // Code 500: Server error
                connection->Send(std::move(error_response));
                return;
            }
        } else {
            std::cerr << "[CreateLobbyHandler][ERROR] LobbyPlayerService not available." << std::endl;
            auto error_response = network::CreateCreateLobbyResponsePacket<PacketType>(
                500);  // Code 500: Server error
            connection->Send(std::move(error_response));
            return;
        }

        // Envoyer une réponse avec l'ID du lobby
        auto response = network::CreateCreateLobbyResponsePacket<PacketType>(
            200, lobby.id);  // Code 200: Success, include lobby ID
        connection->Send(std::move(response));

        std::cout << "[CreateLobbyHandler] Lobby created successfully: " << lobby.name
                  << " (ID: " << lobby.id << ")" << std::endl;
    } else {
        std::cerr << "[CreateLobbyHandler][ERROR] LobbyService not available." << std::endl;
        auto error_response = network::CreateCreateLobbyResponsePacket<PacketType>(
            500);  // Code 500: Server error
        connection->Send(std::move(error_response));
    }
}

}

#endif  // CREATE_LOBBY_HANDLER_HPP_
