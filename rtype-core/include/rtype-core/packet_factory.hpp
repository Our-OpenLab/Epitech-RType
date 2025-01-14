#ifndef NETWORK_PACKET_FACTORY_HPP_
#define NETWORK_PACKET_FACTORY_HPP_

#include <optional>
#include <network/protocol.hpp>

#include "protocol.hpp"

namespace network {

template <typename PacketType>
Packet<PacketType> CreateRegisterResponsePacket(int status_code);

template <typename PacketType>
Packet<PacketType> CreateLoginResponsePacket(int status_code);

template <typename PacketType>
Packet<PacketType> CreatePrivateMessagePacket(
    std::uint32_t sender_id,
    std::uint32_t recipient_id,
    const std::string& message,
    std::uint64_t message_id,
    std::uint64_t timestamp);

template <typename PacketType>
Packet<PacketType> CreateChannelMessagePacket(
    std::uint32_t sender_id,
    std::uint32_t channel_id,
    const std::string& message,
    std::uint64_t message_id,
    std::uint64_t timestamp);

template <typename PacketType>
Packet<PacketType> CreatePrivateMessageResponsePacket(int status_code);

template <typename PacketType>
Packet<PacketType> CreateChannelMessageResponsePacket(int status_code);

template <typename PacketType>
Packet<PacketType> CreatePlayerReadyPacketResponse(int status_code);

template <typename PacketType>
Packet<PacketType> CreateGetUserListResponsePacket(
    int status_code,
    const std::vector<packets::GetUserListResponsePacket::UserInfo>& users);

template <typename PacketType>
Packet<PacketType> CreatePrivateChatHistoryResponsePacket(
    int status_code,
    const std::vector<packets::PrivateChatHistoryResponsePacket::MessageInfo>& messages);

}  // namespace network

#include "packet_factory.tpp"

#endif  // NETWORK_PACKET_FACTORY_HPP_
