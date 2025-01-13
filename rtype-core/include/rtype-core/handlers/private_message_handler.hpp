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

    if (packet.body.size() != sizeof(network::packets::PrivateMessagePacket)) {
        std::cerr << "[PrivateMessageHandler][ERROR] Invalid PrivateMessagePacket size." << std::endl;
        auto error_response =
            network::CreatePrivateMessageResponsePacket<PacketType>(
                400);  // Code 400: Invalid request
        connection->Send(std::move(error_response));
        return;
    }

    const int sender_id = connection->GetId();
    if (!game_state.IsPlayerActive(sender_id)) {
        std::cerr << "[PrivateMessageHandler][ERROR] Sender is not connected: ID "
                  << sender_id << std::endl;
        auto error_response =
            network::CreatePrivateMessageResponsePacket<PacketType>(
                401);  // Code 401: Unauthorized
        connection->Send(std::move(error_response));
        return;
    }

    const auto* message_data =
        reinterpret_cast<const network::packets::PrivateMessagePacket*>(
            packet.body.data());
    const std::uint32_t recipient_id = message_data->recipient_id;
    const std::string message_content(message_data->message);

    if (const auto message_service = service_container.GetMessageService()) {
        const auto saved_message = message_service->SaveMessage(
            sender_id, recipient_id, std::nullopt, message_content);

        if (!saved_message.has_value()) {
            std::cerr << "[PrivateMessageHandler][ERROR] Failed to save message to database." << std::endl;
            auto error_response =
                network::CreatePrivateMessageResponsePacket<PacketType>(
                    500);  // Code 500: Internal Server Error
            connection->Send(std::move(error_response));
            return;
        }

        const auto& timestamp = saved_message->sent_at;

        auto enriched_packet = network::CreatePrivateMessagePacket<PacketType>(
            sender_id, recipient_id, message_content, saved_message->id,
            timestamp);

        connection->Send(enriched_packet);

        if (sender_id != static_cast<int>(recipient_id)) {
            if (const auto recipient_connection =
                    game_state.GetPlayerConnection(recipient_id)) {
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

}

#endif  // PRIVATE_MESSAGE_HANDLER_HPP_
