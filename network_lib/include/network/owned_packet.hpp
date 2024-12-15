#ifndef OWNED_PACKET_HPP_
#define OWNED_PACKET_HPP_

#include <variant>

#include "protocol.hpp"

namespace network {

template <typename PacketType>
using OwnedPacket = std::variant<OwnedPacketTCP<PacketType>, OwnedPacketUDP<PacketType>>;

}

#endif // OWNED_PACKET_HPP_
