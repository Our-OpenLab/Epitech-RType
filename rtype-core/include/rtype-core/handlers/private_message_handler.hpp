#ifndef PRIVATE_MESSAGE_HANDLER_HPP_
#define PRIVATE_MESSAGE_HANDLER_HPP_

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
void HandlePrivateMessage(const std::shared_ptr<void>& raw_event,
                          ServiceContainer& service_container,
                          GameState<PacketType>& game_state) {
    auto event = std::static_pointer_cast<
        std::pair<network::Packet<PacketType>,
                  std::shared_ptr<network::TcpServerConnection<PacketType>>>>(raw_event);

    const auto& packet = event->first;
    const auto& connection = event->second;

    // Vérifier la taille du paquet
    if (packet.body.size() != sizeof(network::packets::PrivateMessagePacket)) {
        std::cerr << "[PrivateMessageHandler][ERROR] Invalid PrivateMessagePacket size." << std::endl;
        auto error_response =
            network::CreatePrivateMessageResponsePacket<PacketType>(
                400);  // Code 400: Invalid request
        connection->Send(std::move(error_response));
        return;
    }

    // Récupérer l'ID utilisateur depuis l'ID de connexion
    const int connection_id = connection->GetId();
    const auto sender_db_id_opt = game_state.GetDbIdByConnectionId(connection_id);

    if (!sender_db_id_opt.has_value()) {
        std::cerr << "[PrivateMessageHandler][ERROR] Sender is not connected: Connection ID "
                  << connection_id << std::endl;
        auto error_response =
            network::CreatePrivateMessageResponsePacket<PacketType>(
                401);  // Code 401: Unauthorized
        connection->Send(std::move(error_response));
        return;
    }

    const int sender_db_id = sender_db_id_opt.value();

    // Extraire les données du message
    const auto* message_data =
        reinterpret_cast<const network::packets::PrivateMessagePacket*>(packet.body.data());
    const std::uint32_t recipient_id = message_data->recipient_id;
    const std::string message_content(message_data->message);

    // Vérifier que le service de message est disponible
    if (const auto message_service = service_container.GetMessageService()) {
        const auto saved_message = message_service->SaveMessage(
            sender_db_id, recipient_id, message_content);

        if (!saved_message.has_value()) {
            std::cerr << "[PrivateMessageHandler][ERROR] Failed to save message to database." << std::endl;
            auto error_response =
                network::CreatePrivateMessageResponsePacket<PacketType>(
                    500);  // Code 500: Internal Server Error
            connection->Send(std::move(error_response));
            return;
        }

        // Création du paquet enrichi avec les métadonnées du message
        const auto& timestamp = saved_message->sent_at;
        auto enriched_packet = network::CreatePrivateMessagePacket<PacketType>(
            sender_db_id, recipient_id, message_content, saved_message->id,
            timestamp);

        // Envoyer la confirmation au sender
        connection->Send(enriched_packet);

        // Envoyer au destinataire si connecté
        if (sender_db_id != static_cast<int>(recipient_id)) {
            if (const auto recipient_connection =
                    game_state.GetPlayerConnectionByDbId(recipient_id)) {
                recipient_connection->Send(std::move(enriched_packet));
            } else {
                std::cout << "[PrivateMessageHandler] Recipient " << recipient_id
                          << " is not connected. Message stored." << std::endl;
            }
        }
    } else {
        std::cerr << "[PrivateMessageHandler][ERROR] MessageService not available." << std::endl;
        auto error_response =
            network::CreatePrivateMessageResponsePacket<PacketType>(
                500);  // Code 500: Internal Server Error
        connection->Send(std::move(error_response));
    }
}

}  // namespace rtype

#endif  // PRIVATE_MESSAGE_HANDLER_HPP_
