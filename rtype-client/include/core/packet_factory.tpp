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
std::optional<Packet<PacketType>> CreateRegisterPacket(const std::string& username, const std::string& password) {
  if (username.size() > sizeof(packets::RegisterPacket::username) ||
      password.size() > sizeof(packets::RegisterPacket::password)) {
    std::cerr << "[CreateRegisterPacket][ERROR] Username or password too long." << std::endl;
    return std::nullopt;
  }

  packets::RegisterPacket buffer {};
  std::memcpy(buffer.username, username.data(), username.size());
  std::memcpy(buffer.password, password.data(), password.size());

  return PacketFactory<PacketType>::CreatePacket(
        PacketType::kUserRegister,
        std::span(reinterpret_cast<uint8_t*>(&buffer), sizeof(buffer))
    );
}

template <typename PacketType>
std::optional<Packet<PacketType>> CreateLoginPacket(const std::string& username, const std::string& password) {
  if (username.size() > sizeof(packets::LoginPacket::username) ||
      password.size() > sizeof(packets::LoginPacket::password)) {
    std::cerr << "[CreateLoginPacket][ERROR] Username or password too long." << std::endl;
    return std::nullopt;
  }

  packets::LoginPacket buffer {};
  std::memcpy(buffer.username, username.data(), username.size());
  std::memcpy(buffer.password, password.data(), password.size());

  return PacketFactory<PacketType>::CreatePacket(
        PacketType::kUserLogin,
        std::span(reinterpret_cast<uint8_t*>(&buffer), sizeof(buffer))
    );
}

template <typename PacketType>
std::optional<Packet<PacketType>> CreatePrivateMessagePacket(const int recipient_id, const std::string& message_content) {
  if (message_content.size() > sizeof(packets::PrivateMessagePacket::message)) {
    std::cerr << "[CreateMessagePacket][ERROR] Message is too long to send." << std::endl;
    return std::nullopt;
  }

  packets::PrivateMessagePacket message_packet = {};
  message_packet.recipient_id = recipient_id;
  std::memcpy(message_packet.message, message_content.data(), message_content.size());

  return PacketFactory<PacketType>::CreatePacket(
        PacketType::kPrivateMessage,
        std::span(reinterpret_cast<uint8_t*>(&message_packet), sizeof(message_packet))
    );
}

template <typename PacketType>
std::optional<Packet<PacketType>> CreateCreateLobbyPacket(const std::string& name, const std::optional<std::string>& password) {
  if (name.size() > sizeof(packets::CreateLobbyPacket::name) ||
      (password && password->size() > sizeof(packets::CreateLobbyPacket::password))) {
    std::cerr << "[CreateLobbyPacket][ERROR] Name or password too long." << std::endl;
    return std::nullopt;
      }

  packets::CreateLobbyPacket buffer {};
  std::memcpy(buffer.name, name.data(), name.size());
  if (password) {
    std::memcpy(buffer.password, password->data(), password->size());
  }

  return PacketFactory<PacketType>::CreatePacket(
      PacketType::kCreateLobby,
      std::span(reinterpret_cast<uint8_t*>(&buffer), sizeof(buffer))
  );
}

template <typename PacketType>
std::optional<Packet<PacketType>> CreateJoinLobbyPacket(
    const int lobby_id, const std::optional<std::string>& password) {
  if (password && password->size() > sizeof(packets::JoinLobbyPacket::password)) {
    std::cerr << "[CreateJoinLobbyPacket][ERROR] Password too long." << std::endl;
    return std::nullopt;
  }

  packets::JoinLobbyPacket buffer {};
  buffer.lobby_id = lobby_id;
  if (password) {
    std::memcpy(buffer.password, password->data(), password->size());
  }

  return PacketFactory<PacketType>::CreatePacket(
      PacketType::kJoinLobby,
      std::span(reinterpret_cast<uint8_t*>(&buffer), sizeof(buffer))
  );
}

template <typename PacketType>
std::optional<Packet<PacketType>> CreatePlayerReadyPacket(const bool is_ready) {
  packets::PlayerReadyPacket buffer {};
  buffer.is_ready = is_ready;

  return PacketFactory<PacketType>::CreatePacket(
      PacketType::kPlayerReady,
      std::span(reinterpret_cast<uint8_t*>(&buffer), sizeof(buffer))
  );
}

template <typename PacketType>
std::optional<Packet<PacketType>> CreateGetUserListPacket(const std::uint32_t offset, const std::uint32_t limit) {
  packets::GetUserListPacket buffer {};
  buffer.offset = offset;
  buffer.limit = limit;

  return PacketFactory<PacketType>::CreatePacket(
      PacketType::kGetUserList,
      std::span(reinterpret_cast<uint8_t*>(&buffer), sizeof(buffer))
  );
}

template <typename PacketType>
std::optional<Packet<PacketType>> CreatePrivateChatHistoryPacket(const int user_id) {
  packets::PrivateChatHistoryPacket buffer {};
  buffer.user_id = user_id;

  return PacketFactory<PacketType>::CreatePacket(
      PacketType::kPrivateChatHistory,
      std::span(reinterpret_cast<uint8_t*>(&buffer), sizeof(buffer))
  );
}

template <typename PacketType>
std::optional<Packet<PacketType>> CreateGetLobbyPlayersPacket(const int lobby_id) {
  packets::GetLobbyPlayersPacket buffer {};
  buffer.lobby_id = lobby_id;

  return PacketFactory<PacketType>::CreatePacket(
      PacketType::kGetLobbyPlayers,
      std::span(reinterpret_cast<uint8_t*>(&buffer), sizeof(buffer))
  );
}

template <typename PacketType>
std::optional<Packet<PacketType>> CreateLeaveLobbyPacket() {
  Packet<PacketType> packet;
  packet.header.type = PacketType::kLeaveLobby;

  return packet;
}

template <typename PacketType>
std::optional<Packet<PacketType>> CreateGetLobbyListPacket(
    const std::uint32_t offset, const std::uint32_t limit, const std::string& search_term) {
  if (search_term.size() > sizeof(packets::GetLobbyListPacket::search_term)) {
    std::cerr << "[CreateGetLobbyListPacket][WARNING] Search term truncated to fit the buffer." << std::endl;
  }

  packets::GetLobbyListPacket buffer {};
  buffer.offset = offset;
  buffer.limit = limit;

  // Safely copy the search term, truncating if necessary
  const size_t copy_size = std::min(search_term.size(), sizeof(buffer.search_term));
  std::memcpy(buffer.search_term, search_term.data(), copy_size);

  return PacketFactory<PacketType>::CreatePacket(
      PacketType::kGetLobbyList,
      std::span(reinterpret_cast<uint8_t*>(&buffer), sizeof(buffer))
  );
}

template <typename PacketType>
Packet<PacketType> CreatePingPacket(const std::uint32_t timestamp) {
  packets::PingPacket ping = {timestamp};

  return PacketFactory<PacketType>::CreatePacket(
      PacketType::kPing,
      std::span(reinterpret_cast<uint8_t*>(&ping), sizeof(ping))
  );
}

template <typename PacketType>
std::optional<Packet<PacketType>> CreateUdpPortPacket(const uint16_t udp_port) {
  packets::UdpPortPacket buffer {};
  buffer.udp_port = udp_port;

  return PacketFactory<PacketType>::CreatePacket(
      PacketType::kUdpPort,
      std::span(reinterpret_cast<uint8_t*>(&buffer), sizeof(buffer))
  );
}

template <typename PacketType>
std::optional<Packet<PacketType>> CreatePlayerInputPacket(const u_int8_t player_id, const uint16_t actions, const float dir_x, const float dir_y) {
  packets::PlayerInputPacket buffer {
    player_id,
    actions,
    dir_x,
    dir_y
  };

  return PacketFactory<PacketType>::CreatePacket(
      PacketType::kPlayerInput,
      std::span(reinterpret_cast<uint8_t*>(&buffer), sizeof(buffer))
  );
}

template <typename PacketType>
std::optional<Packet<PacketType>> CreateUdpPortPacket(const uint16_t udp_port, const std::string& private_ip) {
  if (private_ip.size() > sizeof(packets::UdpPortPacket::private_ip)) {
    std::cerr << "[CreateUdpPortPacket][ERROR] Private IP address is too long." << std::endl;
    return std::nullopt;
  }

  packets::UdpPortPacket buffer {};
  buffer.udp_port = udp_port;

  const size_t copy_size = std::min(private_ip.size(), sizeof(buffer.private_ip));
  std::memcpy(buffer.private_ip, private_ip.data(), copy_size);

  return PacketFactory<PacketType>::CreatePacket(
      PacketType::kUdpPort,
      std::span(reinterpret_cast<uint8_t*>(&buffer), sizeof(buffer))
  );
}


}  // namespace network

#endif  // NETWORK_PACKET_FACTORY_TPP_
