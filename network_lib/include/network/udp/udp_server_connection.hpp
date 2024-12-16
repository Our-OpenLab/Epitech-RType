#ifndef NETWORK_UDP_SERVER_CONNECTION_HPP_
#define NETWORK_UDP_SERVER_CONNECTION_HPP_

#include <asio/ip/udp.hpp>
#include <asio/io_context.hpp>
#include <iostream>
#include <memory>

#include "network/concurrent_queue.hpp"
#include "network/owned_packet.hpp"
#include "udp_connection.hpp"

namespace network {

/**
 * @brief UDP Server Connection class.
 *
 * Specializes the generic UdpConnection class for server use cases,
 * enabling asynchronous sending and receiving of packets for a server.
 *
 * @tparam PacketType The type of packet used in the connection.
 */
template <typename PacketType>
class UdpServerConnection final : public UdpConnection<PacketType>,
                                  public std::enable_shared_from_this<UdpServerConnection<PacketType>> {
 public:
  /**
   * @brief Constructs a UdpServerConnection.
   *
   * @param io_context The Asio I/O context for managing asynchronous operations.
   * @param port The local port to bind the UDP socket.
   * @param received_queue The queue where received packets will be pushed.
   */
  explicit UdpServerConnection(asio::io_context& io_context, uint16_t port,
                                ConcurrentQueue<OwnedPacket<PacketType>>& received_queue)
      : UdpConnection<PacketType>(io_context, port), received_queue_(received_queue) {}

  /**
   * @brief Destructor to clean up resources.
   *
   * Ensures the socket is properly closed.
   */
  ~UdpServerConnection() override = default;

 protected:
  /**
   * @brief Receives a packet from the UDP socket.
   *
   * Overrides the receive logic to push packets to the received queue.
   */
  void OnPacketReceived(Packet<PacketType> packet) override {
    received_queue_.Push(OwnedPacket<PacketType>(
        OwnedPacketUDP<PacketType>{std::move(this->remote_endpoint_), std::move(packet)}));
  }

  /**
   * @brief Returns a shared pointer to this instance.
   *
   * Used internally for safe asynchronous operations.
   *
   * @return A shared pointer to this instance.
   */
  std::shared_ptr<UdpConnection<PacketType>> ThisShared() override {
    return std::static_pointer_cast<UdpServerConnection<PacketType>>(this->shared_from_this());
  }

 private:
  ConcurrentQueue<OwnedPacket<PacketType>>& received_queue_;
};

}  // namespace network

#endif  // NETWORK_UDP_SERVER_CONNECTION_HPP_
