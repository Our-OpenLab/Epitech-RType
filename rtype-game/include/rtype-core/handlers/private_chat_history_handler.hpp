#ifndef PRIVATE_CHAT_HISTORY_HANDLER_HPP_
#define PRIVATE_CHAT_HISTORY_HANDLER_HPP_

#include <iostream>
#include <memory>
#include <utility>
#include <vector>

#include "rtype-core/service_container.hpp"
#include "rtype-core/game_state.hpp"
#include "rtype-core/protocol.hpp"

namespace rtype {

template <typename PacketType>
void HandlePrivateChatHistory(const std::shared_ptr<void>& raw_event,
                              ServiceContainer& service_container,
                              GameState<PacketType>& game_state) {
    auto event = std::static_pointer_cast<
        std::pair<network::Packet<PacketType>,
                  std::shared_ptr<network::TcpServerConnection<PacketType>>>>(raw_event);

    const auto& packet = event->first;
    const auto& connection = event->second;

    if (packet.body.size() != sizeof(network::packets::PrivateChatHistoryPacket)) {
        std::cerr << "[PrivateChatHistoryHandler][ERROR] Invalid PrivateChatHistoryPacket size." << std::endl;
        return;
    }

    const auto* request = reinterpret_cast<const network::packets::PrivateChatHistoryPacket*>(packet.body.data());
    const auto target_user_id = request->user_id;

    const int connection_id = connection->GetId();
    const auto player_db_id_opt = game_state.GetDbIdByConnectionId(connection_id);

    if (!player_db_id_opt) {
        std::cerr << "[PrivateChatHistoryHandler][ERROR] Player is not connected: Connection ID " << connection_id << std::endl;
        auto response_packet = network::CreatePrivateChatHistoryResponsePacket<PacketType>(
            401, {});  // Code 401: Unauthorized
        connection->Send(std::move(response_packet));
        return;
    }

    const auto sender_db_id = *player_db_id_opt;

    const auto message_service = service_container.GetMessageService();
    if (!message_service) {
        std::cerr << "[PrivateChatHistoryHandler][ERROR] MessageService not available." << std::endl;
        auto response_packet = network::CreatePrivateChatHistoryResponsePacket<PacketType>(
            500, {});  // Code 500: Internal server error
        connection->Send(std::move(response_packet));
        return;
    }

    const auto messages = message_service->GetMessages(sender_db_id, target_user_id);

    if (messages.empty()) {
        std::cerr << "[PrivateChatHistoryHandler][INFO] No chat history found for user: " << target_user_id << "." << std::endl;
        auto response_packet = network::CreatePrivateChatHistoryResponsePacket<PacketType>(
            404, {});  // Code 404: Not Found
        connection->Send(std::move(response_packet));
        return;
    }

    std::vector<network::packets::PrivateChatHistoryResponsePacket::MessageInfo> message_info_list;
    for (const auto& message : messages) {
        network::packets::PrivateChatHistoryResponsePacket::MessageInfo message_info{};
        message_info.sender_id = message.sender_id;
        std::strncpy(message_info.message, message.content.c_str(), sizeof(message_info.message) - 1);
        message_info.message_id = message.id;
        message_info.timestamp = message.sent_at;
        message_info_list.push_back(message_info);
    }

    auto response_packet = network::CreatePrivateChatHistoryResponsePacket<PacketType>(
        200, message_info_list);  // Code 200: Success
    connection->Send(std::move(response_packet));

    std::cout << "[PrivateChatHistoryHandler] Sent chat history response for user " << target_user_id << "." << std::endl;
}

}  // namespace rtype

#endif  // PRIVATE_CHAT_HISTORY_HANDLER_HPP_
