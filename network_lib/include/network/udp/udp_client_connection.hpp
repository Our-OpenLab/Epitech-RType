#ifndef NETWORK_UDP_CLIENT_CONNECTION_HPP_
#define NETWORK_UDP_CLIENT_CONNECTION_HPP_

#include <asio/io_context.hpp>
#include <asio/ip/udp.hpp>
#include <array>
#include <iostream>
#include <memory>
#include <utility>

#include "network/concurrent_queue.hpp"
#include "network/protocol.hpp"
#include "udp_connection.hpp"

namespace network {

/**
 * @brief UDP Client Connection class.
 *
 * Specializes the generic UdpConnection class for client use cases,
 * enabling asynchronous sending and receiving of packets for a client.
 *
 * @tparam PacketType The type of packet used in the connection.
 */
template <typename PacketType>
class UdpClientConnection final : public UdpConnection<PacketType>,
                                  public std::enable_shared_from_this<UdpClientConnection<PacketType>>
 {
 public:
  /**
   * @brief Constructs a UdpClientConnection.
   *
   * @param io_context The Asio I/O context for managing asynchronous operations.
   * @param received_queue The queue where received packets will be pushed.
   */
  explicit UdpClientConnection(asio::io_context& io_context,
                                ConcurrentQueue<Packet<PacketType>>& received_queue)
      : UdpConnection<PacketType>(io_context, 0), received_queue_(received_queue) {
    try {
      std::cout << "[Client][UDP][INFO] UDP socket opened on port "
                << this->socket_.local_endpoint().port() << std::endl;
    } catch (const std::exception& e) {
      std::cerr << "[Client][UDP][ERROR] Failed to open UDP socket: " << e.what() << std::endl;
      throw;
    }
  }

  /**
   * @brief Destructor to clean up resources.
   *
   * Ensures the socket is properly closed.
   */
  ~UdpClientConnection() override = default;

  /**
   * @brief Gets the local port of the UDP socket.
   *
   * @return The port number the socket is bound to.
   */
  [[nodiscard]] uint16_t GetLocalPort() const {
    return this->socket_.local_endpoint().port();
  }

 protected:
  /**
   * @brief Receives a packet from the UDP socket.
   *
   * Overrides the receive logic to push packets to the received queue.
   */
  void OnPacketReceived(Packet<PacketType> packet) override {
    received_queue_.Push(std::move(packet));
  }

  /**
   * @brief Returns a shared pointer to this instance.
   *
   * Used internally for safe asynchronous operations.
   *
   * @return A shared pointer to this instance.
   */
  std::shared_ptr<UdpConnection<PacketType>> ThisShared() override {
    return std::static_pointer_cast<UdpClientConnection<PacketType>>(this->shared_from_this());
  }

 private:
  ConcurrentQueue<Packet<PacketType>>& received_queue_;
};

}  // namespace network

#endif  // NETWORK_UDP_CLIENT_CONNECTION_HPP_
