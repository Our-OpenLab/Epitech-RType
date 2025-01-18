#ifndef NETWORK_PACKET_FACTORY_TPP_
#define NETWORK_PACKET_FACTORY_TPP_

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <optional>

#include <network/protocol.hpp>

#include "protocol.hpp"

namespace network {

template <typename PacketType>
Packet<PacketType> CreateRegisterResponsePacket(const int status_code) {
  packets::RegisterResponsePacket response = {status_code};

  return PacketFactory<PacketType>::CreatePacket(
        PacketType::kUserRegisterResponse,
        std::span(reinterpret_cast<uint8_t*>(&response), sizeof(response))
    );
}

template <typename PacketType>
Packet<PacketType> CreateLoginResponsePacket(const int status_code) {
  packets::LoginResponsePacket response = {status_code};

  return PacketFactory<PacketType>::CreatePacket(
        PacketType::kUserLoginResponse,
        std::span(reinterpret_cast<uint8_t*>(&response), sizeof(response))
    );
}

template <typename PacketType>
Packet<PacketType> CreatePrivateMessagePacket(
    const std::uint32_t sender_id,
    const std::uint32_t recipient_id,
    const std::string& message,
    const std::uint64_t message_id,
    const std::uint64_t timestamp) {
  packets::PrivateChatMessagePacket packet = {};
  packet.sender_id = sender_id;
  packet.recipient_id = recipient_id;
  packet.message_id = message_id;
  packet.timestamp = timestamp;
  std::memcpy(packet.message, message.data(), message.size());

  return PacketFactory<PacketType>::CreatePacket(
        PacketType::kPrivateChatMessage,
        std::span(reinterpret_cast<uint8_t*>(&packet), sizeof(packet))
    );
}

template <typename PacketType>
Packet<PacketType> CreateChannelMessagePacket(
    const std::uint32_t sender_id,
    const std::uint32_t channel_id,
    const std::string& message,
    const std::uint64_t message_id,
    const std::uint64_t timestamp) {
  packets::ChannelChatMessagePacket packet = {};
  packet.sender_id = sender_id;
  packet.channel_id = channel_id;
  packet.message_id = message_id;
  packet.timestamp = timestamp;
  std::memcpy(packet.message, message.data(), message.size());

  return PacketFactory<PacketType>::CreatePacket(
        PacketType::kChannelChatMessage,
        std::span(reinterpret_cast<uint8_t*>(&packet), sizeof(packet))
    );
}

template <typename PacketType>
Packet<PacketType> CreatePrivateMessageResponsePacket(const int status_code) {
    packets::PrivateMessageResponsePacket error_packet = {
        .status_code = status_code
    };

    return PacketFactory<PacketType>::CreatePacket(
        PacketType::kPrivateMessageResponse,
        std::span(reinterpret_cast<uint8_t*>(&error_packet), sizeof(error_packet))
    );
}

template <typename PacketType>
Packet<PacketType> CreateChannelMessageResponsePacket(const int status_code) {
    packets::ChannelMessageResponsePacket error_packet = {
        .status_code = status_code
    };

    return PacketFactory<PacketType>::CreatePacket(
        PacketType::kChannelMessageResponse,
        std::span(reinterpret_cast<uint8_t*>(&error_packet), sizeof(error_packet))
    );
}

template <typename PacketType>
Packet<PacketType> CreateCreateLobbyResponsePacket(const int status_code, const int lobby_id) {
    packets::CreateLobbyResponsePacket response = {
        .status_code = status_code,
        .lobby_id = lobby_id
    };

    return PacketFactory<PacketType>::CreatePacket(
         PacketType::kCreateLobbyResponse,
         std::span(reinterpret_cast<uint8_t*>(&response), sizeof(response))
     );
}

template <typename PacketType>
Packet<PacketType> CreatePlayerReadyPacketResponse(const int status_code) {
    packets::PlayerReadyPacketResponse error_packet = {
        .status_code = status_code
    };

    return PacketFactory<PacketType>::CreatePacket(
        PacketType::kPlayerReadyResponse,
        std::span(reinterpret_cast<uint8_t*>(&error_packet), sizeof(error_packet))
    );
}

template <typename PacketType>
Packet<PacketType> CreateGetUserListResponsePacket(
    int status_code,
    const std::vector<packets::GetUserListResponsePacket::UserInfo>& users) {

    Packet<PacketType> packet;
    packet.header.type = PacketType::kGetUserListResponse;

    packet.Push(status_code);

    if (!users.empty()) {
        const size_t user_data_size = users.size() * sizeof(packets::GetUserListResponsePacket::UserInfo);
        size_t current_size = packet.body.size();
        packet.body.resize(current_size + user_data_size);
        std::memcpy(packet.body.data() + current_size, users.data(), user_data_size);
        packet.header.size += user_data_size;
    }

    return packet;
}

template <typename PacketType>
Packet<PacketType> CreatePrivateChatHistoryResponsePacket(
    int status_code,
    const std::vector<packets::PrivateChatHistoryResponsePacket::MessageInfo>& messages) {

    Packet<PacketType> packet;
    packet.header.type = PacketType::kPrivateChatHistoryResponse;

    // Ajouter le code de statut
    packet.Push(status_code);

    // Ajouter les messages, si disponibles
    if (!messages.empty()) {
        const size_t message_data_size = messages.size() * sizeof(packets::PrivateChatHistoryResponsePacket::MessageInfo);
        size_t current_size = packet.body.size();
        packet.body.resize(current_size + message_data_size);
        std::memcpy(packet.body.data() + current_size, messages.data(), message_data_size);
        packet.header.size += message_data_size;
    }

    return packet;
}

template <typename PacketType>
Packet<PacketType> CreateGetLobbyPlayersResponsePacket(
    int status_code,
    const std::vector<packets::GetLobbyPlayersResponsePacket::PlayerInfo>& players) {

    Packet<PacketType> packet;
    packet.header.type = PacketType::kGetLobbyPlayersResponse;

    packet.Push(status_code);

    if (!players.empty()) {
        const size_t player_data_size = players.size() * sizeof(packets::GetLobbyPlayersResponsePacket::PlayerInfo);
        size_t current_size = packet.body.size();
        packet.body.resize(current_size + player_data_size);
        std::memcpy(packet.body.data() + current_size, players.data(), player_data_size);
        packet.header.size += player_data_size;
    }

    return packet;
}

template <typename PacketType>
Packet<PacketType> CreateLeaveLobbyResponsePacket(const int status_code) {
    packets::LeaveLobbyResponsePacket response = {};
    response.status_code = status_code;

    return PacketFactory<PacketType>::CreatePacket(
        PacketType::kLeaveLobbyResponse,
        std::span(reinterpret_cast<uint8_t*>(&response), sizeof(response))
    );
}

template <typename PacketType>
Packet<PacketType> CreatePlayerLeftLobbyNotificationPacket(const int player_id) {
    packets::PlayerLeftLobbyPacket notification = {};
    notification.player_id = player_id;

    return PacketFactory<PacketType>::CreatePacket(
        PacketType::kPlayerLeftLobby,
        std::span(reinterpret_cast<uint8_t*>(&notification), sizeof(notification))
    );
}

template <typename PacketType>
Packet<PacketType> CreateJoinLobbyResponsePacket(const int status_code) {
    packets::JoinLobbyResponsePacket response = {};
    response.status_code = status_code;

    return PacketFactory<PacketType>::CreatePacket(
        PacketType::kJoinLobbyResponse,
        std::span(reinterpret_cast<uint8_t*>(&response), sizeof(response))
    );
}

template <typename PacketType>
Packet<PacketType> CreatePlayerJoinedLobbyPacket(const int player_id, const std::string& username) {
    packets::PlayerJoinedLobbyPacket packet = {};
    packet.player_id = player_id;

    std::memcpy(packet.username, username.data(), std::min(username.size(), sizeof(packet.username)));

    return PacketFactory<PacketType>::CreatePacket(
        PacketType::kPlayerJoinedLobby,
        std::span(reinterpret_cast<uint8_t*>(&packet), sizeof(packet))
    );
}

template <typename PacketType>
Packet<PacketType> CreateGetLobbyListResponsePacket(
    int status_code,
    const std::vector<packets::GetLobbyListResponsePacket::LobbyInfo>& lobbies) {
    Packet<PacketType> packet;
    packet.header.type = PacketType::kGetLobbyListResponse;

    packet.Push(status_code);

    if (!lobbies.empty()) {
        const size_t lobby_data_size = lobbies.size() * sizeof(packets::GetLobbyListResponsePacket::LobbyInfo);
        size_t current_size = packet.body.size();
        packet.body.resize(current_size + lobby_data_size);
        std::memcpy(packet.body.data() + current_size, lobbies.data(), lobby_data_size);
        packet.header.size += lobby_data_size;
    }

    return packet;
}

template <typename PacketType>
Packet<PacketType> CreateLobbyPlayerReadyPacket(const int player_id, const bool is_ready) {
    packets::LobbyPlayerReadyPacket notification = {};
    notification.player_id = player_id;
    notification.is_ready = is_ready;

    return PacketFactory<PacketType>::CreatePacket(
        PacketType::kLobbyPlayerReady,
        std::span(reinterpret_cast<uint8_t*>(&notification), sizeof(notification))
    );
}

template <typename PacketType>
Packet<PacketType> CreateGameConnectionInfoPacket(
    const std::string& ip_address,
    const std::vector<int>& ports) {
    packets::GameConnectionInfoPacket packet = {};

    std::memcpy(packet.ip_address, ip_address.data(),
                std::min(ip_address.size(), sizeof(packet.ip_address)));

    const size_t port_count = std::min(ports.size(), static_cast<size_t>(std::size(packet.ports)));
    std::copy_n(ports.begin(), port_count, packet.ports);

    return PacketFactory<PacketType>::CreatePacket(
        PacketType::kGameConnectionInfo,
        std::span(reinterpret_cast<uint8_t*>(&packet), sizeof(packet))
    );
}

}  // namespace network

#endif  // NETWORK_PACKET_FACTORY_TPP_
