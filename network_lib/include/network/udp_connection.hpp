#ifndef CONNECTION_UDP_HPP
#define CONNECTION_UDP_HPP

#include "concurrent_queue.hpp"

#include "owned_packet.hpp"

namespace network {

template <typename PacketType>
class UdpConnection final : public std::enable_shared_from_this<UdpConnection<PacketType>> {
 public:
  explicit UdpConnection(asio::io_context& io_context, const uint16_t port,
                         ConcurrentQueue<OwnedPacket<PacketType>>& received_queue)
      : socket_(io_context, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)),
        received_queue_(received_queue) {}

  ~UdpConnection() {
    close();
  }

  [[nodiscard]] bool is_open() const { return socket_.is_open(); }

  [[nodiscard]] uint16_t get_local_port() const { return socket_.local_endpoint().port(); }

  void start() {
    auto self = this->shared_from_this();
    asio::post(socket_.get_executor(), [self]() {
        self->receive_from();
    });
  }

  void send_to(const Packet<PacketType>& packet,
               const asio::ip::udp::endpoint& endpoint) {
    if (packet.size() > 1472) {  // 1500 (MTU) - 28 (Header size for UDP/IPv4)
      std::cerr << "[UDP][ERROR] Packet size exceeds MTU limit" << std::endl;
      return;
    }

    auto buffer = packet.data();

    socket_.async_send_to(
        asio::buffer(buffer), endpoint,
        [buffer = std::move(buffer)](const std::error_code& ec,
                                     const std::size_t bytes_transferred) {
          if (ec) {
            std::cerr << "[UDP][ERROR] Failed to send packet: " << ec.message()
                      << std::endl;
          } else {
            std::cout << "[UDP][INFO] Packet sent (" << bytes_transferred
                      << " bytes)." << std::endl;
          }
        });
  }

  void receive_from() {
    socket_.async_receive_from(
        asio::buffer(recv_buffer_), remote_endpoint_,
        [this](const std::error_code& ec, const std::size_t bytes_received) {
          if (!ec && bytes_received > 0) {
            if (bytes_received < sizeof(Header<PacketType>)) {
              std::cerr << "[UDP][ERROR] Packet size is too small" << std::endl;
            } else {
              Packet<PacketType> packet;

              std::memcpy(&packet.header, recv_buffer_.data(),
                          sizeof(Header<PacketType>));

              if (bytes_received > sizeof(Header<PacketType>)) {
                packet.body.assign(
                    recv_buffer_.begin() + sizeof(Header<PacketType>),
                    recv_buffer_.begin() + bytes_received);
              }

              received_queue_.push(OwnedPacket<PacketType>(
                OwnedPacketUDP<PacketType>{std::move(remote_endpoint_), std::move(packet)}));
            }
            receive_from();
          } else {
            std::cerr << "[UDP][ERROR] Receive error: " << ec.message()
                      << std::endl;
          }
        });
  }

  void close() {
    asio::post(socket_.get_executor(), [self = this->shared_from_this()]() {
        if (self->socket_.is_open()) {
            self->socket_.close();
        }
    });
  }

 private:
  asio::ip::udp::socket socket_;
  asio::ip::udp::endpoint remote_endpoint_;
  static constexpr size_t RECV_BUFFER_SIZE = 4096;
  std::array<char, RECV_BUFFER_SIZE> recv_buffer_{};
  ConcurrentQueue<OwnedPacket<PacketType>>& received_queue_;
};

}  // namespace network

#endif  // CONNECTION_UDP_HPP
