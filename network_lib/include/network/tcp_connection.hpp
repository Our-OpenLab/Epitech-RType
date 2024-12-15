#ifndef CONNECTION_TCP_HPP_
#define CONNECTION_TCP_HPP_

#include <asio.hpp>

#include "concurrent_queue.hpp"
#include "tcp_connection_interface.hpp"
#include "protocol.hpp"

namespace network {

template <typename PacketType>
class TcpConnection : public TcpConnectionInterface<PacketType>  {
 public:
  explicit TcpConnection(asio::ip::tcp::socket socket)
      : socket_(std::move(socket)) {}

  ~TcpConnection() override = default;

  void disconnect() override {
    asio::post(socket_.get_executor(), [self = this->this_shared()]() {
      if (self->socket_.is_open()) {
        self->socket_.close();
      }
    });
  }

  [[nodiscard]] bool is_connected() const override { return socket_.is_open(); }

  [[nodiscard]] asio::ip::tcp::socket& get_socket() { return socket_; }

  void send(const Packet<PacketType>& packet) override {
    auto self = this_shared();
    asio::post(socket_.get_executor(), [self, packet]() {
      const bool is_writing = !self->send_queue_.empty();
      self->send_queue_.push(packet);
      if (!is_writing) {
        self->write_packet();
      }
    });
  }

  void send(Packet<PacketType>&& packet) override {
    auto self = this_shared();
    asio::post(socket_.get_executor(), [self, packet = std::move(packet)]() mutable {
      const bool is_writing = !self->send_queue_.empty();
      self->send_queue_.push(std::move(packet));
      if (!is_writing) {
        self->write_packet();
      }
    });
  }

 protected:
  virtual std::shared_ptr<TcpConnection> this_shared() = 0;

  virtual void read_header() = 0;
  virtual void read_body() = 0;

  virtual void write_packet() {
    if (Packet<PacketType> current_packet;
        this->send_queue_.try_pop(current_packet)) {

      auto buffer = current_packet.data();

      asio::async_write(
          this->socket_, asio::buffer(buffer),
          [this, buffer = std::move(buffer)](const std::error_code& ec,
                                             std::size_t /*length*/) mutable {
              if (!ec) {
                 if (!this->send_queue_.empty()) {
                   this->write_packet();
                 }
              } else {
                std::cerr << "[TCP] Error writing packet: " << ec.message() << std::endl;
                this->disconnect();
              }
          });
    }
  }

  static constexpr size_t MAX_BODY_SIZE = 1024 * 1024;

  asio::ip::tcp::socket socket_;
  ConcurrentQueue<Packet<PacketType>> send_queue_{};
  Packet<PacketType> incoming_packet_{};
};

}  // namespace network

#endif  // CONNECTION_TCP_HPP_
