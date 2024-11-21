#ifndef SERVER_CONNECTION_HPP_
#define SERVER_CONNECTION_HPP_

#include "connection.hpp"

namespace network {
class ServerConnection : public Connection {
 public:
  ServerConnection(asio::io_context& io_context, asio::ip::tcp::socket socket,
                   ConcurrentQueue<OwnedPacket>& received_queue,
                   const uint32_t id)
      : Connection(io_context, std::move(socket)),
        received_queue_(received_queue),
        connection_id_(id) {}

  void start() {
    read_header();
  }

  [[nodiscard]] uint32_t get_id() const { return connection_id_; }

  ~ServerConnection() override { Connection::disconnect(); }

 protected:
  void read_header() override {
    asio::async_read(
        socket_, asio::buffer(&incoming_packet_.header, sizeof(Header)),
        [this](const std::error_code& ec, std::size_t /*length*/) {
          if (!ec) {
            if (incoming_packet_.header.size > 0) {
              if (incoming_packet_.header.size > MAX_BODY_SIZE) {
                handle_error("Invalid Body Size", asio::error::message_size);
                return;
              }
              incoming_packet_.body.resize(incoming_packet_.header.size);
              read_body();
            } else {
              received_queue_.push({shared_from_this(), incoming_packet_});
              read_header();
            }
          } else {
            handle_error("Read Header", ec);
          }
        });
  }

  void read_body() override {
    asio::async_read(
        socket_,
        asio::buffer(incoming_packet_.body.data(),
                     incoming_packet_.header.size),
        [this](const std::error_code& ec, std::size_t /*length*/) {
          if (!ec) {
            received_queue_.push({shared_from_this(), incoming_packet_});
            read_header();
          } else {
            handle_error("Read Body", ec);
          }
        });
  }

  void write_header() override {
    Packet current_packet;

    if (send_queue_.try_pop(current_packet)) {
      asio::async_write(
          socket_, asio::buffer(&current_packet.header, sizeof(Header)),
          [this, self = shared_from_this(),
           current_packet = std::move(current_packet)](
              const std::error_code& ec, std::size_t /*length*/) mutable {
            if (!ec) {
              if (current_packet.header.size > 0) {
                write_body(std::move(current_packet));
              } else {
                if (!send_queue_.empty()) {
                  write_header();
                }
              }
            } else {
              handle_error("Write Header", ec);
            }
          });
    }
  }

  void write_body(Packet current_packet) override {
    asio::async_write(
        socket_,
        asio::buffer(current_packet.body.data(), current_packet.body.size()),
        [this, self = shared_from_this()](const std::error_code& ec,
                                          std::size_t /*length*/) {
          if (!ec) {
            if (!send_queue_.empty()) {
              write_header();
            }
          } else {
            handle_error("Write Body", ec);
          }
        });
  }

 private:
  void handle_error(const std::string& context, const std::error_code& ec) {
    std::cerr << '[' << connection_id_ << "] Context: " << context
              << " | Error Code: " << ec.value()
              << " | Message: " << ec.message() << '\n';
    disconnect();
  }

  ConcurrentQueue<OwnedPacket>& received_queue_;

  uint32_t connection_id_;
};
}  // namespace network

#endif  // SERVER_CONNECTION_HPP_
