#ifndef NETWORK_TCP_CONNECTION_HPP_
#define NETWORK_TCP_CONNECTION_HPP_

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
  ~TcpConnection() override { Disconnect(); }

  /**
   * @brief Disconnects the connection.
   *
   * Shuts down and closes the socket to release resources.
   */
  void Disconnect() override {
    try {
      if (socket_.is_open()) {
        socket_.shutdown(asio::ip::tcp::socket::shutdown_both);
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
    auto self = ThisShared();
    asio::post(socket_.get_executor(), [self, packet]() {
      const bool is_writing = !self->send_queue_.Empty();
      self->send_queue_.Push(packet);
      if (!is_writing) {
        self->WritePacket();
      }
    });
  }

  /**
   * @brief Sends a packet over the connection (rvalue overload).
   *
   * @param packet The packet to send (moved).
   */
  void Send(Packet<PacketType>&& packet) override {
    auto self = ThisShared();
    asio::post(socket_.get_executor(), [self, packet = std::move(packet)]() mutable {
      const bool is_writing = !self->send_queue_.Empty();
      self->send_queue_.Push(std::move(packet));
      if (!is_writing) {
        self->WritePacket();
      }
    });
  }

 protected:
  /**
   * @brief Returns a shared pointer to the derived class.
   *
   * Used for safe asynchronous operations.
   */
  virtual std::shared_ptr<TcpConnection> ThisShared() = 0;

  /**
   * @brief Reads the header of an incoming packet.
   *
   * Implemented in derived classes.
   */
  virtual void ReadHeader() = 0;

  /**
   * @brief Reads the body of an incoming packet.
   *
   * Implemented in derived classes.
   */
  virtual void ReadBody() = 0;

  /**
   * @brief Writes a packet from the send queue to the socket.
   */
  void WritePacket() {
    if (Packet<PacketType> current_packet;
        send_queue_.TryPop(current_packet)) {
      auto buffer = current_packet.Data();
      asio::async_write(
          socket_, asio::buffer(buffer),
          [this, buffer = std::move(buffer)](const std::error_code& ec, std::size_t /*length*/) {
            if (!ec) {
              if (!send_queue_.Empty()) {
                WritePacket();
              }
            } else {
              std::cerr << "[TCP][ERROR] Error writing packet: " << ec.message() << std::endl;
              Disconnect();
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

#endif  // NETWORK_TCP_CONNECTION_HPP_
