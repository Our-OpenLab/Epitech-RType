#ifndef CONNECTION_HPP_
#define CONNECTION_HPP_

#include <asio.hpp>

#include "protocol.hpp"
#include "concurrent_queue.hpp"
#include "connection_interface.hpp"

namespace network {

template <typename PacketType>
class Connection : public ConnectionInterface<PacketType> {
 public:
  explicit Connection(asio::io_context& io_context,
                      asio::ip::tcp::socket socket)
      : io_context_(io_context), socket_(std::move(socket)) {}

  ~Connection() override = default;

  void disconnect() override {
    asio::post(this->io_context_, [self = this->this_shared()]() {
      if (self->socket_.is_open()) {
        self->socket_.close();
      }
    });
  }

  [[nodiscard]] bool is_connected() const override { return socket_.is_open(); }

  void send(const Packet<PacketType>& packet) override {
    auto self = this_shared();
    asio::post(io_context_, [self, packet]() {
        const bool is_writing = !self->send_queue_.empty();
        self->send_queue_.push(packet);
        if (!is_writing) {
            self->write_packet();
        }
    });
  }

  void send(Packet<PacketType>&& packet) override {
    auto self = this_shared();
    asio::post(io_context_, [self, packet = std::move(packet)]() mutable {
      const bool is_writing = !self->send_queue_.empty();
      self->send_queue_.push(std::move(packet));
      if (!is_writing) {
        self->write_packet();
      }
    });
  }

 protected:
  virtual std::shared_ptr<Connection> this_shared() = 0;

  asio::io_context& io_context_;
  asio::ip::tcp::socket socket_;
  ConcurrentQueue<Packet<PacketType>> send_queue_{};
  Packet<PacketType> incoming_packet_;

  virtual void read_header() = 0;
  virtual void read_body() = 0;
  virtual void write_packet() = 0;

  static constexpr size_t MAX_BODY_SIZE = 1024 * 1024;  // 1 MB
};
}  // namespace network

#endif  // CONNECTION_HPP_
