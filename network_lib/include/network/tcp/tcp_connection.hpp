#ifndef TCP_CONNECTION_HPP_
#define TCP_CONNECTION_HPP_

#include <asio.hpp>
#include <memory>
#include <iostream>

#include "network/concurrent_queue.hpp"
#include "network/protocol.hpp"
#include "tcp_connection_interface.hpp"

namespace network {

/**
 * @brief TCP Connection base class.
 *
 * Provides common functionality for managing a TCP connection, such as
 * sending packets, reading headers/bodies, and handling disconnects.
 *
 * @tparam PacketType The type of packet used in the connection.
 */
template <typename PacketType>
class TcpConnection : public TcpConnectionInterface<PacketType> {
 public:
  /**
   * @brief Constructor for TcpConnection.
   *
   * @param socket The socket associated with this connection.
   */
  explicit TcpConnection(asio::ip::tcp::socket socket)
      : socket_(std::move(socket)) {}

  /**
   * @brief Destructor for TcpConnection.
   *
   * Ensures the connection is properly disconnected.
   */
  ~TcpConnection() override { TcpConnection::Disconnect(); }

  /**
   * @brief Disconnects the connection.
   *
   * Shuts down and closes the socket to release resources.
   */
  void Disconnect() override {
    try {
      if (socket_.is_open()) {
        socket_.close();
      }
    } catch (const std::exception& e) {
      std::cerr << "[TCP][ERROR] Failed to disconnect: " << e.what() << std::endl;
    }
  }

  /**
   * @brief Checks if the connection is active.
   *
   * @return `true` if the connection is active, `false` otherwise.
   */
  [[nodiscard]] bool IsConnected() const override { return socket_.is_open(); }

  /**
   * @brief Sends a packet over the connection.
   *
   * @param packet The packet to send.
   */
  void Send(const Packet<PacketType>& packet) override {
    auto self = ThisShared(); // Keep a strong reference
    asio::post(socket_.get_executor(), [self, packet]() {
      const bool is_writing = !self->send_queue_.Empty();
      self->send_queue_.Push(packet);
      if (!is_writing) {
        self->WritePacket(self);
      }
    });
  }

  /**
   * @brief Sends a packet over the connection (rvalue overload).
   *
   * @param packet The packet to send (moved).
   */
  void Send(Packet<PacketType>&& packet) override {
    auto self = ThisShared(); // Keep a strong reference
    asio::post(socket_.get_executor(), [self, packet = std::move(packet)]() mutable {
      const bool is_writing = !self->send_queue_.Empty();
      self->send_queue_.Push(std::move(packet));
      if (!is_writing) {
        self->WritePacket(self);
      }
    });
  }

 protected:
  /**
   * @brief Returns a shared pointer to the derived class.
   *
   * Used for safe asynchronous operations.
   *
   * Derived classes must override this to return:
   *    return this->shared_from_this();
   */
  virtual std::shared_ptr<TcpConnection> ThisShared() = 0;

  /**
  * @brief Reads the header of an incoming packet.
  *
  * @param self A shared pointer to this connection.
  */
  void ReadHeader(std::shared_ptr<TcpConnection> self) {
    asio::async_read(
        self->socket_,
        asio::buffer(&self->incoming_packet_.header, sizeof(Header<PacketType>)),
        [self](const std::error_code& ec, std::size_t) {
          if (!ec) {
            if (self->incoming_packet_.header.size > 0) {
              if (self->incoming_packet_.header.size > TcpConnection<PacketType>::kMaxBodySize) {
                std::cerr << "[TCP][ERROR] Invalid body size in header." << std::endl;
                self->Disconnect();
                return;
              }
              self->incoming_packet_.body.resize(self->incoming_packet_.header.size);
              self->ReadBody(self);
            } else {
              self->OnPacketReceived(std::move(self->incoming_packet_));
              self->ReadHeader(self);
            }
          } else {
            std::cerr << "[TCP][ERROR] Error reading header: " << ec.message() << std::endl;
            self->Disconnect();
          }
        });
  }

  /**
  * @brief Reads the body of an incoming packet.
  *
  * @param self A shared pointer to this connection.
  */
  void ReadBody(std::shared_ptr<TcpConnection> self) {
    asio::async_read(
        self->socket_,
        asio::buffer(self->incoming_packet_.body.data(), self->incoming_packet_.header.size),
        [self](const std::error_code& ec, std::size_t) {
          if (!ec) {
            self->OnPacketReceived(std::move(self->incoming_packet_));
            self->ReadHeader(self);
          } else {
            std::cerr << "[TCP][ERROR] Error reading body: " << ec.message() << std::endl;
            self->Disconnect();
          }
        });
  }

  /**
   * @brief Callback when a packet is received.
   *
   * Derived classes must override this method to handle received packets.
   *
   * @param packet The received packet.
   */
  virtual void OnPacketReceived(Packet<PacketType>&& packet) = 0;

  /**
   * @brief Writes a packet from the send queue to the socket.
   *
   * @param self A shared pointer to this connection (ensures lifetime).
   */
  void WritePacket(std::shared_ptr<TcpConnection> self) {
    Packet<PacketType> current_packet;
    if (self->send_queue_.TryPop(current_packet)) {
      auto buffer = current_packet.Data();

      // Capture 'self' to keep the object alive during async_write
      asio::async_write(
          self->socket_,
          asio::buffer(buffer),
          [self, buffer = std::move(buffer)](const std::error_code& ec, std::size_t /*length*/) {
            if (!ec) {
              // If there's more data to send, write the next packet
              if (!self->send_queue_.Empty()) {
                self->WritePacket(self);
              }
            } else {
              std::cerr << "[TCP][ERROR] Error writing packet: " << ec.message() << std::endl;
              self->Disconnect();
            }
          });
    }
  }

  asio::ip::tcp::socket socket_;
  ConcurrentQueue<Packet<PacketType>> send_queue_{};
  Packet<PacketType> incoming_packet_{};

  static constexpr size_t kMaxBodySize = 1024 * 1024;  // 1 MB
};

}  // namespace network

#endif  // TCP_CONNECTION_HPP_
