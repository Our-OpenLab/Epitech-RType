#ifndef SERVER_CONNECTION_HPP_
#define SERVER_CONNECTION_HPP_

#include "connection.hpp"
#include "protocol.hpp"

namespace network {

template <typename PacketType>
class ServerConnection : public Connection<PacketType>, public std::enable_shared_from_this<ServerConnection<PacketType>> {
 public:
  ServerConnection(asio::io_context& io_context, asio::ip::tcp::socket socket,
                   ConcurrentQueue<OwnedPacket<PacketType>>& received_queue,
                   const uint32_t id)
      : Connection<PacketType>(io_context, std::move(socket)),
        received_queue_(received_queue),
        connection_id_(id) {}

  void start() {
    auto self = this->shared_from_this();
    asio::post(self->io_context_, [self]() {
        self->read_header();
    });
  }

  [[nodiscard]] uint32_t get_id() const { return connection_id_; }

 protected:

  std::shared_ptr<Connection<PacketType>> this_shared() override {
    return this->shared_from_this();
  }

  void read_header() override {
    asio::async_read(
        this->socket_,
        asio::buffer(&this->incoming_packet_.header, sizeof(Header<PacketType>)),
        [this](const std::error_code& ec, std::size_t /*length*/) {
            if (!ec) {
                if (this->incoming_packet_.header.size > 0) {
                    if (this->incoming_packet_.header.size > this->MAX_BODY_SIZE) {
                        handle_error("Invalid Body Size", asio::error::message_size);
                        return;
                    }
                    this->incoming_packet_.body.resize(this->incoming_packet_.header.size);
                    read_body();
                } else {
                    received_queue_.push({this->shared_from_this(), this->incoming_packet_});
                    asio::post(this->io_context_, [this]() { read_header(); });
                }
            } else {
                handle_error("Read Header", ec);
            }
        });
  }

  void read_body() override {
    asio::async_read(
        this->socket_,
        asio::buffer(this->incoming_packet_.body.data(), this->incoming_packet_.header.size),
        [this](const std::error_code& ec, std::size_t /*length*/) {
          if (!ec) {
            received_queue_.push({this->shared_from_this(), this->incoming_packet_});
            read_header();
          } else {
            handle_error("Read Body", ec);
          }
        });
  }

  void write_packet() override {
    Packet<PacketType> current_packet;

    if (this->send_queue_.try_pop(current_packet)) {
      std::vector<asio::const_buffer> buffers;

      buffers.push_back(asio::buffer(&current_packet.header, sizeof(current_packet.header)));

      if (!current_packet.body.empty()) {
        buffers.push_back(asio::buffer(current_packet.body.data(), current_packet.body.size()));
      }

      asio::async_write(
          this->socket_, buffers,
          [this, current_packet = std::move(current_packet)](const std::error_code& ec, std::size_t /*length*/) mutable {
              if (!ec) {
                 if (!this->send_queue_.empty()) {
                   this->write_packet();
                 }
              } else {
                  this->handle_error("Write Packet", ec);
              }
          });
    }
  }

 private:
  void handle_error(const std::string& context, const std::error_code& ec) {
    std::cerr << '[' << connection_id_ << "] Context: " << context
              << " | Error Code: " << ec.value()
              << " | Message: " << ec.message() << '\n';
    this->disconnect();
  }

  ConcurrentQueue<OwnedPacket<PacketType>>& received_queue_;

  uint32_t connection_id_;
};
}  // namespace network

#endif  // SERVER_CONNECTION_HPP_
