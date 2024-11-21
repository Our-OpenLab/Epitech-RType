#ifndef CONNECTION_HPP_
#define CONNECTION_HPP_

#include <asio.hpp>

#include "concurrent_queue.hpp"
#include "connection_interface.hpp"

namespace network {
class Connection : public ConnectionInterface,
                   public std::enable_shared_from_this<Connection> {
 public:
  explicit Connection(asio::io_context& io_context,
                      asio::ip::tcp::socket socket)
      : io_context_(io_context), socket_(std::move(socket)) {}

  ~Connection() override = 0;

  void disconnect() override {
    asio::post(io_context_, [this]() {
      if (socket_.is_open()) {
        socket_.close();
      }
    });
  }

  [[nodiscard]] bool is_connected() const override { return socket_.is_open(); }

  bool send(const Packet& packet) override {
    asio::post(io_context_, [this, packet]() {
      const bool is_writing = !send_queue_.empty();
      send_queue_.push(packet);
      if (!is_writing) {
        write_header();
      }
    });
    return true;
  }

  bool send(Packet&& packet) override {
    asio::post(io_context_, [this, packet = std::move(packet)]() mutable {
      const bool is_writing = !send_queue_.empty();
      send_queue_.push(std::move(packet));
      if (!is_writing) {
        write_header();
      }
    });
    return true;
  }

 protected:
  asio::io_context& io_context_;
  asio::ip::tcp::socket socket_;
  ConcurrentQueue<Packet> send_queue_{};
  Packet incoming_packet_;

  virtual void read_body() = 0;
  virtual void read_header() = 0;
  virtual void write_header() = 0;
  virtual void write_body(Packet current_packet) = 0;

  static constexpr size_t MAX_BODY_SIZE = 1024 * 1024;  // 1 MB
};
}  // namespace network

#endif  // CONNECTION_HPP_
