#ifndef NETWORK_TCP_SERVER_CONNECTION_HPP_
#define NETWORK_TCP_SERVER_CONNECTION_HPP_

#include <asio.hpp>
#include <memory>
#include <iostream>

#include "tcp_connection.hpp"
#include "network/owned_packet.hpp"

namespace network {

/**
 * @brief TCP Server Connection class.
 *
 * Manages a single client connection for the server.
 *
 * @tparam PacketType The type of packet used in the connection.
 */
template <typename PacketType>
class TcpServerConnection final : public TcpConnection<PacketType>,
                                  public std::enable_shared_from_this<TcpServerConnection<PacketType>> {
 public:
  /**
   * @brief Constructs a TcpServerConnection.
   *
   * @param socket The TCP socket for the connection.
   * @param received_queue The queue where received packets will be pushed.
   * @param id The unique ID of the client connection.
   */
  TcpServerConnection(asio::ip::tcp::socket socket,
                      ConcurrentQueue<OwnedPacket<PacketType>>& received_queue,
                      const uint32_t id)
      : TcpConnection<PacketType>(std::move(socket)),
        received_queue_(received_queue),
        connection_id_(id) {}

  /**
   * @brief Starts handling the connection.
   *
   * This method initializes the connection by posting a task to the ASIO event loop
   * and begins reading incoming data.
   */
  void Start() {
    // Capture a local shared_ptr once
    auto self = this->shared_from_this();

    // Post a small task to let ASIO run this on its event loop
    asio::post(this->socket_.get_executor(),
               [self]() {
                 self->ReadHeader(self);
               });
  }

  /**
   * @brief Gets the unique ID of the connection.
   *
   * The ID is assigned by the server to uniquely identify the client connection.
   *
   * @return The unique ID of the connection.
   */
  [[nodiscard]] uint32_t GetId() const { return connection_id_; }

  /**
   * @brief Access the underlying TCP socket.
   *
   * Provides access to the TCP socket for advanced configurations or diagnostics.
   *
   * @return A constant reference to the TCP socket.
   */
  [[nodiscard]] const asio::ip::tcp::socket& GetSocket() const {
    return TcpConnection<PacketType>::socket_;
  }

protected:
  /**
   * @brief Returns a shared pointer to the current instance.
   *
   * Used internally for safe asynchronous operations and lifecycle management.
   *
   * @return A shared pointer to this `TcpServerConnection` instance.
   */
  std::shared_ptr<TcpConnection<PacketType>> ThisShared() override {
    return this->shared_from_this();
  }

  /**
   * @brief Callback invoked when a fully received packet is ready.
   *
   * This method processes the received packet by pushing it into the shared queue
   * for further handling by the server.
   *
   * @param self A shared pointer to this connection instance.
   * @param packet The fully received packet.
   */
  void OnPacketReceived(Packet<PacketType>&& packet) override {
    received_queue_.Push(
                OwnedPacket<PacketType>(OwnedPacketTCP<PacketType>{
                    this->shared_from_this(), packet}));
  }

private:
  /**
   * @brief Queue to store received packets.
   *
   * This queue is shared with the server to manage incoming packets
   * from this client connection.
   */
  ConcurrentQueue<OwnedPacket<PacketType>>& received_queue_;

  /**
   * @brief The unique ID of this connection.
   *
   * Used by the server to identify and manage the connection.
   */
  uint32_t connection_id_;
};

}  // namespace network

#endif  // NETWORK_TCP_SERVER_CONNECTION_HPP_
