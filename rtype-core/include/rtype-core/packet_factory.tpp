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
Packet<PacketType> CreateCreateLobbyResponsePacket(const int status_code) {
    packets::CreateLobbyResponsePacket error_packet = {
        .status_code = status_code
    };

    return PacketFactory<PacketType>::CreatePacket(
        PacketType::kCreateLobbyResponse,
        std::span(reinterpret_cast<uint8_t*>(&error_packet), sizeof(error_packet))
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

}  // namespace network

#endif  // NETWORK_PACKET_FACTORY_TPP_
