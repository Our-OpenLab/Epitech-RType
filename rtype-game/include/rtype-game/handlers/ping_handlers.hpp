#ifndef PING_HANDLER_HPP_
#define PING_HANDLER_HPP_

#include <iostream>
#include <memory>
#include <cstring>

#include "rtype-game/service_container.hpp"
#include "rtype-game/protocol.hpp"

namespace rtype {

/**
 * @brief Handles incoming Ping packets and responds with a Pong packet.
 *
 * This function processes Ping packets sent by clients to measure latency.
 * It sends back a Pong packet with the same timestamp included in the Ping packet.
 *
 * @tparam PacketType The type of the packet protocol being used.
 * @param raw_event The raw event containing the packet and connection information.
 */
template <typename PacketType>
void HandlePingTCPPacket(const std::shared_ptr<void>& raw_event) {
    auto event = std::static_pointer_cast<
        std::pair<network::Packet<PacketType>,
                  std::shared_ptr<network::TcpServerConnection<PacketType>>>>(raw_event);

    const auto& packet = event->first;
    const auto& connection = event->second;

    if (packet.body.size() != sizeof(network::packets::PingPacket)) {
        std::cerr << "[PingHandler][ERROR] Invalid PingPacket size." << std::endl;
        return;
    }

    const auto* ping_packet = reinterpret_cast<const network::packets::PingPacket*>(packet.body.data());
    const uint32_t timestamp = ping_packet->timestamp;

    network::packets::PongPacket pong_packet = {timestamp};
    auto pong_response = network::PacketFactory<PacketType>::CreatePacket(
        PacketType::kPong,
        std::span(reinterpret_cast<uint8_t*>(&pong_packet), sizeof(pong_packet))
    );

    connection->Send(std::move(pong_response));
}

}  // namespace rtype

#endif  // PING_HANDLER_HPP_
