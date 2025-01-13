#ifndef NETWORK_PACKET_FACTORY_HPP_
#define NETWORK_PACKET_FACTORY_HPP_

#include <optional>
#include <network/protocol.hpp>

#include "protocol.hpp"

namespace network {

/**
 * @brief Creates a Packet for user registration.
 *
 * This function constructs the packet directly, filling the body with
 * the `RegisterPacket` data.
 *
 * @tparam PacketType Enum type for packet identification.
 * @param username The username (max 31 characters).
 * @param password The password (max 31 characters).
 * @return An optional Packet containing the registration data. Returns
 *         std::nullopt if the username or password exceeds the maximum size.
 */
template <typename PacketType>
std::optional<Packet<PacketType>> CreateRegisterPacket(const std::string& username, const std::string& password);

template <typename PacketType>
std::optional<Packet<PacketType>> CreateLoginPacket(const std::string& username, const std::string& password);

template <typename PacketType>
std::optional<Packet<PacketType>> CreatePrivateMessagePacket(int recipient_id, const std::string& message_content);

template <typename PacketType>
std::optional<Packet<PacketType>> CreateCreateLobbyPacket(const std::string& name, const std::optional<std::string>& password);

template <typename PacketType>
std::optional<Packet<PacketType>> CreateJoinLobbyPacket(int lobby_id, const std::optional<std::string>& password);

template <typename PacketType>
std::optional<Packet<PacketType>> CreatePlayerReadyPacket(bool is_ready);

}  // namespace network

#include "packet_factory.tpp"

#endif  // NETWORK_PACKET_FACTORY_HPP_
