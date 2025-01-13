#ifndef NETWORK_TCP_CONNECTION_INTERFACE_HPP_
#define NETWORK_TCP_CONNECTION_INTERFACE_HPP_

#include "network/protocol.hpp"

namespace network {

/**
 * @brief Interface for TCP connection operations.
 *
 * Defines the basic contract for managing TCP connections,
 * including checking connection status, disconnecting, and sending packets.
 *
 * @tparam PacketType The type of packet to be sent or received.
 */
template <typename PacketType>
class TcpConnectionInterface {
public:
  /**
   * @brief Virtual destructor for TcpConnectionInterface.
   *
   * Ensures proper cleanup of derived classes.
   */
  virtual ~TcpConnectionInterface() = default;

  /**
   * @brief Checks if the connection is active.
   *
   * @return `true` if the connection is active and ready for communication,
   *         `false` otherwise.
   */
  [[nodiscard]] virtual bool IsConnected() const = 0;

  /**
   * @brief Disconnects the connection.
   *
   * Ensures the connection is properly closed and any associated
   * resources are released. It is safe to call this method multiple times.
   */
  virtual void Disconnect() = 0;

  /**
   * @brief Sends a packet over the connection.
   *
   * @param packet A constant reference to the packet to be sent.
   * This method enqueues the packet for sending.
   */
  virtual void Send(const Packet<PacketType>& packet) = 0;

  /**
   * @brief Sends a packet over the connection (rvalue overload).
   *
   * @param packet An rvalue reference to the packet to be sent.
   * This method enqueues the packet for sending.
   */
  virtual void Send(Packet<PacketType>&& packet) = 0;
};

}  // namespace network

#endif  // NETWORK_TCP_CONNECTION_INTERFACE_HPP_
