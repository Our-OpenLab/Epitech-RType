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
                      uint32_t id)
      : TcpConnection<PacketType>(std::move(socket)),
        received_queue_(received_queue),
        connection_id_(id) {}

  /**
   * @brief Starts handling the connection.
   *
   * Begins the asynchronous process to read incoming data from the client.
   */
  void Start() {
    auto self = this->shared_from_this();
    asio::post(this->socket_.get_executor(), [self]() { self->ReadHeader(); });
  }

  /**
   * @brief Gets the unique ID of the connection.
   *
   * The connection ID is assigned by the server and can be used to identify
   * a specific client connection.
   *
   * @return The connection ID.
   */
  [[nodiscard]] uint32_t GetId() const { return connection_id_; }

  /**
   * @brief Provides access to the underlying TCP socket.
   *
   * This can be used for advanced operations on the socket, such as modifying
   * socket options.
   *
   * @return A reference to the TCP socket.
   */
  [[nodiscard]] const asio::ip::tcp::socket& GetSocket() const { return TcpConnection<PacketType>::socket_; }

 protected:
  /**
   * @brief Returns a shared pointer to the current instance.
   *
   * Used internally to maintain the lifetime of the connection object during
   * asynchronous operations.
   *
   * @return A shared pointer to this `TcpServerConnection` instance.
   */
  std::shared_ptr<TcpConnection<PacketType>> ThisShared() override {
    return this->shared_from_this();
  }

  /**
   * @brief Asynchronously reads the packet header from the client.
   *
   * This function initiates an asynchronous read operation to retrieve
   * the packet header from the client.
   */
  void ReadHeader() override {
    asio::async_read(
        this->socket_, asio::buffer(&this->incoming_packet_.header, sizeof(Header<PacketType>)),
        [this](const std::error_code& ec, std::size_t /*length*/) {
          if (!ec) {
            if (this->incoming_packet_.header.size > 0) {
              if (this->incoming_packet_.header.size > TcpConnection<PacketType>::kMaxBodySize) {
                std::cerr << "[TCP Server] Invalid body size in header." << std::endl;
                this->Disconnect();
                return;
              }
              this->incoming_packet_.body.resize(this->incoming_packet_.header.size);
              ReadBody();
            } else {
              received_queue_.Push(OwnedPacket<PacketType>(
                  OwnedPacketTCP<PacketType>{this->shared_from_this(), this->incoming_packet_}));
              ReadHeader();
            }
          } else {
            std::cerr << "[TCP Server] Error reading header: " << ec.message() << std::endl;
            this->Disconnect();
          }
        });
  }

  /**
   * @brief Asynchronously reads the packet body from the client.
   *
   * This function initiates an asynchronous read operation to retrieve
   * the body of the packet after the header has been successfully read.
   */
  void ReadBody() override {
    asio::async_read(
        this->socket_,
        asio::buffer(this->incoming_packet_.body.data(), this->incoming_packet_.header.size),
        [this](const std::error_code& ec, std::size_t /*length*/) {
          if (!ec) {
            received_queue_.Push(OwnedPacket<PacketType>(
                OwnedPacketTCP<PacketType>{this->shared_from_this(), this->incoming_packet_}));
            ReadHeader();
          } else {
            std::cerr << "[TCP Server] Error reading body: " << ec.message() << std::endl;
            this->Disconnect();
          }
        });
  }

 private:
  /**
   * @brief Queue to store received packets.
   *
   * The queue is shared with the server and is used to manage
   * incoming packets from this client.
   */
  ConcurrentQueue<OwnedPacket<PacketType>>& received_queue_;

  /**
   * @brief The unique ID of this connection.
   *
   * Assigned by the server to identify this client connection.
   */
  uint32_t connection_id_;
};

}  // namespace network

#endif  // NETWORK_TCP_SERVER_CONNECTION_HPP_
