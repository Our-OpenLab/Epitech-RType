#ifndef NETWORK_UDP_CONNECTION_INTERFACE_HPP_
#define NETWORK_UDP_CONNECTION_INTERFACE_HPP_

#include "network/protocol.hpp"

namespace network {

/**
 * @brief Interface for UDP connection operations.
 *
 * Defines the basic contract for managing UDP connections,
 * including methods for checking connection status, closing connections,
 * sending, and receiving packets.
 *
 * @tparam PacketType The type of packet to be sent or received.
 */
template <typename PacketType>
class UdpConnectionInterface {
 public:
  /**
   * @brief Virtual destructor for UdpConnectionInterface.
   *
   * Ensures proper cleanup of derived classes.
   */
  virtual ~UdpConnectionInterface() = default;

  /**
   * @brief Checks if the connection is open.
   *
   * @return `true` if the connection is open, `false` otherwise.
   */
  [[nodiscard]] virtual bool IsOpen() const = 0;

  /**
   * @brief Closes the connection.
   *
   * Implementations should ensure resources are properly released.
   */
  virtual void Close() = 0;

  /**
   * @brief Sends a packet to a specific endpoint.
   *
   * This method is used to send a packet using the UDP connection to a given
   * remote endpoint.
   *
   * @param packet The packet to send.
   * @param endpoint The target UDP endpoint for the packet.
   */
  virtual void SendTo(const Packet<PacketType>& packet,
                       const asio::ip::udp::endpoint& endpoint) = 0;

  /**
   * @brief Sends a packet to a specific endpoint (rvalue overload).
   *
   * This method is used to send a packet using the UDP connection to a given
   * remote endpoint. The packet is passed by rvalue reference to allow for
   * move semantics.
   *
   * @param packet The packet to send (moved).
   * @param endpoint The target UDP endpoint for the packet.
   */
  virtual void SendTo(Packet<PacketType>&& packet,
                       const asio::ip::udp::endpoint& endpoint) = 0;

  /**
   * @brief Starts listening for incoming packets.
   *
   * This method is responsible for initiating the asynchronous reception
   * of packets.
   */
  virtual void Start() = 0;

  /**
   * @brief Receives packets from the connection.
   *
   * Implementations should provide the logic to handle incoming packets
   * and ensure they are processed or queued appropriately.
   */
  virtual void ReceiveFrom() = 0;
};

}  // namespace network

#endif  // NETWORK_UDP_CONNECTION_INTERFACE_HPP_
