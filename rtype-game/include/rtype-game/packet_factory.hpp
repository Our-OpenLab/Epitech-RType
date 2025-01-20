#ifndef NETWORK_PACKET_FACTORY_HPP_
#define NETWORK_PACKET_FACTORY_HPP_

#include <optional>
#include <network/protocol.hpp>

#include "protocol.hpp"

namespace network {

template <typename PacketType>
Packet<PacketType> CreatePingPacket(std::uint32_t timestamp);

template <typename PacketType>
Packet<PacketType> CreatePongPacket(std::uint32_t timestamp);

}  // namespace network

#include "packet_factory.tpp"

#endif  // NETWORK_PACKET_FACTORY_HPP_
