#ifndef MESSAGE_DISPATCHER_HPP_
#define MESSAGE_DISPATCHER_HPP_

#include <array>
#include <functional>
#include <memory>
#include <network/owned_packet.hpp>
#include <network/tcp/tcp_server_connection.hpp>

#include "event_queue.hpp"

namespace network {

/**
 * @brief The MessageDispatcher class is responsible for routing incoming packets
 * to their appropriate handlers based on packet type.
 *
 * The class decouples packet handling logic from the core networking functionality,
 * allowing flexibility in handling various TCP and UDP packet types. It uses an
 * EventQueue to publish events for further processing.
 *
 * @tparam PacketType Enum type representing the different packet types.
 */
template <typename PacketType>
class MessageDispatcher {
public:
  /**
   * @brief Function signature for handling TCP packets.
   *
   * Each handler receives a Packet object and the associated TCP connection.
   */
  using TCPHandler = std::function<void(Packet<PacketType>&&, const std::shared_ptr<TcpServerConnection<PacketType>>&)>;

  /**
   * @brief Function signature for handling UDP packets.
   *
   * Each handler receives a Packet object and the associated UDP endpoint.
   */
  using UDPHandler = std::function<void(Packet<PacketType>&&, const asio::ip::udp::endpoint&)>;

  /**
   * @brief Constructs a MessageDispatcher with a reference to an EventQueue.
   *
   * @param event_queue Reference to the EventQueue used for publishing events.
   */
  explicit MessageDispatcher(rtype::EventQueue& event_queue);

  /**
   * @brief Dispatches an OwnedPacket to the appropriate handler.
   *
   * Depending on whether the packet is TCP or UDP, the corresponding handler is invoked.
   * If no handler is registered for the packet type, a default handler is called.
   *
   * @param owned_packet The packet to be dispatched.
   */
  void Dispatch(OwnedPacket<PacketType>&& owned_packet);

private:
  /**
   * @brief Handles TCP packets by invoking the appropriate handler.
   *
   * @param packet_variant The OwnedPacketTCP containing the packet and its connection.
   */
  void HandlePacket(OwnedPacketTCP<PacketType>&& packet_variant) const;

  /**
   * @brief Handles UDP packets by invoking the appropriate handler.
   *
   * @param packet_variant The OwnedPacketUDP containing the packet and its endpoint.
   */
  void HandlePacket(OwnedPacketUDP<PacketType>&& packet_variant) const;

  /**
   * @brief Default handler for unrecognized or unregistered TCP packets.
   *
   * @param packet The unhandled TCP packet.
   * @param connection The associated TCP connection.
   */
  void DefaultTCPHandler(Packet<PacketType>&& packet, const std::shared_ptr<TcpServerConnection<PacketType>>& connection) const;

  /**
   * @brief Default handler for unrecognized or unregistered UDP packets.
   *
   * @param packet The unhandled UDP packet.
   * @param endpoint The associated UDP endpoint.
   */
  void DefaultUDPHandler(Packet<PacketType>&& packet, const asio::ip::udp::endpoint& endpoint) const;

  /**
   * @brief Initializes the handler arrays with default handlers.
   *
   * Sets all handlers to nullptr initially, allowing later customization by registering handlers.
   */
  void InitializeHandlers();

  rtype::EventQueue& event_queue_; ///< Reference to the EventQueue for publishing events.

  /// Array of handlers for TCP packets, indexed by PacketType.
  std::array<TCPHandler, static_cast<size_t>(PacketType::kMaxTypes)> tcp_handlers_;

  /// Array of handlers for UDP packets, indexed by PacketType.
  std::array<UDPHandler, static_cast<size_t>(PacketType::kMaxTypes)> udp_handlers_;
};

}  // namespace network

#include "message_dispatcher.tpp"

#endif  // MESSAGE_DISPATCHER_HPP_
