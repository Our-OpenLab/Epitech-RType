#ifndef CLIENT_CONNECTION_HPP_
#define CLIENT_CONNECTION_HPP_

#include "connection.hpp"

namespace network {
class ClientConnection : public Connection {
 public:
  ClientConnection(asio::io_context& io_context, asio::ip::tcp::socket socket,
                   ConcurrentQueue<Packet>& received_queue)
      : Connection(io_context, std::move(socket)),
        received_queue_(received_queue) {}

  ~ClientConnection() override { Connection::disconnect(); }

  void connect(const asio::ip::tcp::resolver::results_type& endpoints,
               std::promise<bool>& connection_result) {
    asio::async_connect(
        socket_, endpoints,
        [this, &connection_result](
            const std::error_code& ec,
            const asio::ip::tcp::endpoint& /*endpoint*/) mutable {
          if (!ec) {
            connection_result.set_value(true);
            read_header();
          } else {
            connection_result.set_value(false);
            handle_error("Connect", ec);
          }
        });
  }

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
              received_queue_.push(incoming_packet_);
              read_header();
            }
          } else {
            socket_.close();
          }
        });
  }

  void read_body() override {
    asio::async_read(socket_,
                     asio::buffer(incoming_packet_.body.data(),
                                  incoming_packet_.header.size),
                     [this](const std::error_code& ec, std::size_t /*length*/) {
                       if (!ec) {
                         received_queue_.push(incoming_packet_);
                         read_header();
                       } else {
                         socket_.close();
                       }
                     });
  }

  void write_header() override {
    Packet current_packet;

    if (send_queue_.try_pop(current_packet)) {
      asio::async_write(
          socket_, asio::buffer(&current_packet.header, sizeof(Header)),
          [this, current_packet = std::move(current_packet)](
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
              socket_.close();
            }
          });
    }
  }

  void write_body(Packet current_packet) override {
    asio::async_write(
        socket_,
        asio::buffer(current_packet.body.data(), current_packet.body.size()),
        [this](const std::error_code& ec, std::size_t /*length*/) {
          if (!ec) {
            if (!send_queue_.empty()) {
              write_header();
            }
          } else {
            socket_.close();
          }
        });
  }

 private:
  void handle_error(const std::string& context, const std::error_code& ec) {
    std::cerr << "[Client] " << context << " Error: " << ec.message() << '\n';
    disconnect();
  }

  ConcurrentQueue<Packet>& received_queue_;
};
}  // namespace network

#endif  // CLIENT_CONNECTION_HPP_
