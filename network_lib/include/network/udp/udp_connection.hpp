#ifndef NETWORK_UDP_CONNECTION_HPP_
#define NETWORK_UDP_CONNECTION_HPP_

#include <asio.hpp>
#include <array>
#include <iostream>
#include <memory>
#include <utility>

#include "udp_connection_interface.hpp"
#include "network/protocol.hpp"

namespace network {

/**
 * @brief A generic UDP connection class.
 *
 * Provides common functionality for sending and receiving UDP packets,
 * suitable for both client and server implementations.
 *
 * @tparam PacketType The type of packet used in the connection.
 */
template <typename PacketType>
class UdpConnection : public UdpConnectionInterface<PacketType> {
 public:
  /**
   * @brief Constructs a UDP connection.
   *
   * @param io_context The Asio I/O context for managing asynchronous operations.
   * @param port The local port to bind the UDP socket.
   */
  explicit UdpConnection(asio::io_context& io_context, uint16_t port)
      : socket_(io_context, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)) {
    std::cout << "[UDP][INFO] Socket created on port " << port << std::endl;
  }

  /**
   * @brief Destructor to clean up resources.
   *
   * Ensures the socket is properly closed.
   */
  ~UdpConnection() override { UdpConnection::Close(); }

  /**
   * @brief Checks if the UDP socket is open.
   *
   * @return `true` if the socket is open, `false` otherwise.
   */
  [[nodiscard]] bool IsOpen() const override { return socket_.is_open(); }

  /**
   * @brief Closes the UDP socket.
   *
   * Ensures any resources associated with the socket are released.
   */
  void Close() override {
    try {
      if (socket_.is_open()) {
        socket_.close();
        std::cout << "[UDP][INFO] Socket closed." << std::endl;
      }
    } catch (const std::exception& e) {
      std::cerr << "[UDP][ERROR] Failed to close socket: " << e.what() << std::endl;
    }
  }

  /**
   * @brief Sends a packet to a specified endpoint.
   *
   * @param packet The packet to send.
   * @param endpoint The target UDP endpoint.
   */
  void SendTo(const Packet<PacketType>& packet,
              const asio::ip::udp::endpoint& endpoint) override {
    SendToImpl(packet, endpoint);
  }

  /**
   * @brief Sends a packet to a specified endpoint (rvalue overload).
   *
   * @param packet The packet to send (moved).
   * @param endpoint The target UDP endpoint.
   */
  void SendTo(Packet<PacketType>&& packet,
              const asio::ip::udp::endpoint& endpoint) override {
    SendToImpl(std::move(packet), endpoint);
  }

  /**
   * @brief Starts listening for incoming packets.
   *
   * Asynchronously receives packets and processes them.
   */
  void Start() override {
    auto self = ThisShared();
    asio::post(socket_.get_executor(), [self]() { self->ReceiveFrom(); });
  }

 protected:
  /**
   * @brief Must be implemented by derived classes to return a shared pointer.
   *
   * Ensures the derived class controls its shared instance.
   *
   * @return A shared pointer to the current instance.
   */
  virtual std::shared_ptr<UdpConnection> ThisShared() = 0;

  /**
   * @brief Receives a packet from the UDP socket.
   *
   * Asynchronously handles incoming data and processes it.
   */
  void ReceiveFrom() override {
    socket_.async_receive_from(
        asio::buffer(recv_buffer_), remote_endpoint_,
        [self = ThisShared()](const std::error_code& ec, std::size_t bytes_received) {
          if (!ec && bytes_received > 0) {
            if (bytes_received < sizeof(Header<PacketType>)) {
              std::cerr << "[UDP][ERROR] Packet size is too small." << std::endl;
            } else {
              Packet<PacketType> packet;
              std::memcpy(&packet.header, self->recv_buffer_.data(), sizeof(Header<PacketType>));

              if (bytes_received > sizeof(Header<PacketType>)) {
                packet.body.assign(
                    self->recv_buffer_.begin() + sizeof(Header<PacketType>),
                    self->recv_buffer_.begin() + bytes_received);
              }
              self->OnPacketReceived(std::move(packet));
            }
            self->ReceiveFrom();  // Continue listening for packets.
          } else {
            std::cerr << "[UDP][ERROR] Receive error: " << ec.message() << std::endl;
          }
        });
  }

  /**
   * @brief Internal implementation for sending packets.
   *
   * Handles both lvalue and rvalue packets efficiently.
   *
   * @param packet The packet to send (either lvalue or rvalue).
   * @param endpoint The target UDP endpoint.
   */
  template <typename T>
  void SendToImpl(T&& packet, const asio::ip::udp::endpoint& endpoint) {
    if (packet.Size() > kMaxPacketSize) {
      std::cerr << "[UDP][ERROR] Packet size exceeds MTU limit." << std::endl;
      return;
    }

    auto buffer = packet.Data();
    socket_.async_send_to(
        asio::buffer(buffer), endpoint,
        [buffer = std::move(buffer)](const std::error_code& ec, std::size_t bytes_transferred) {
          if (ec) {
            std::cerr << "[UDP][ERROR] Failed to send packet: " << ec.message() << std::endl;
          } else {
            std::cout << "[UDP][INFO] Packet sent (" << bytes_transferred << " bytes)." << std::endl;
          }
        });
  }

  /**
   * @brief Callback when a packet is received.
   *
   * Derived classes can override this method to handle received packets.
   *
   * @param packet The received packet.
   */
  virtual void OnPacketReceived(Packet<PacketType> packet) {
    std::cout << "[UDP][INFO] Packet received." << std::endl;
  }

  static constexpr size_t kMaxPacketSize = 1472;  // 1500 (MTU) - 28 (UDP/IPv4 header size)
  static constexpr size_t kRecvBufferSize = 4096;

  asio::ip::udp::socket socket_;
  asio::ip::udp::endpoint remote_endpoint_;
  std::array<char, kRecvBufferSize> recv_buffer_{};
};

}  // namespace network

#endif  // NETWORK_UDP_CONNECTION_HPP_
