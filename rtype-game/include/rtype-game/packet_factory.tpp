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
Packet<PacketType> CreatePingPacket(const std::uint32_t timestamp) {
  packets::PingPacket ping = {timestamp};

  return PacketFactory<PacketType>::CreatePacket(
      PacketType::kPing,
      std::span(reinterpret_cast<uint8_t*>(&ping), sizeof(ping))
  );
}

template <typename PacketType>
Packet<PacketType> CreatePongPacket(const std::uint32_t timestamp) {
  packets::PongPacket pong = {timestamp};

  return PacketFactory<PacketType>::CreatePacket(
      PacketType::kPong,
      std::span(reinterpret_cast<uint8_t*>(&pong), sizeof(pong))
  );
}

}  // namespace network

#endif  // NETWORK_PACKET_FACTORY_TPP_
