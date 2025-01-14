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

}  // namespace network

#endif  // NETWORK_PACKET_FACTORY_TPP_
