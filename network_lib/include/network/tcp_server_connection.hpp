#ifndef SERVER_TCP_CONNECTION_HPP_
#define SERVER_TCP_CONNECTION_HPP_

#include "tcp_connection.hpp"

#include "owned_packet.hpp"

namespace network {

template <typename PacketType>
class TcpServerConnection final : public TcpConnection<PacketType>, public std::enable_shared_from_this<TcpServerConnection<PacketType>> {
 public:
  TcpServerConnection(asio::ip::tcp::socket socket,
                   ConcurrentQueue<OwnedPacket<PacketType>>& received_queue,
                   const uint32_t id)
      : TcpConnection<PacketType>(std::move(socket)),
        received_queue_(received_queue), connection_id_(id) {}

  void start() {
    auto self = this->shared_from_this();
    asio::post(this->socket_.get_executor(), [self]() {
        self->read_header();
    });
  }

  [[nodiscard]] uint32_t get_id() const { return connection_id_; }

 protected:

  std::shared_ptr<TcpConnection<PacketType>> this_shared() override {
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
                      std::cerr << "[ServerTCP] Error reading header: Invalid Body Size" << std::endl;
                      this->disconnect();
                      return;
                    }
                    this->incoming_packet_.body.resize(this->incoming_packet_.header.size);
                    read_body();
                } else {
                  received_queue_.push(OwnedPacket<PacketType>(
    			OwnedPacketTCP<PacketType>{this->shared_from_this(), this->incoming_packet_}));
                                        #warning "TODO: Check if this is the correct way to handle this"
                    asio::post(this->socket_.get_executor(), [this]() { read_header(); });
                }
            } else {
              std::cerr << "[ServerTCP] Error reading header: " << ec.message() << std::endl;
              this->disconnect();
            }
        });
  }

  void read_body() override {
    asio::async_read(
        this->socket_,
        asio::buffer(this->incoming_packet_.body.data(), this->incoming_packet_.header.size),
        [this](const std::error_code& ec, std::size_t /*length*/) {
          if (!ec) {
            received_queue_.push(OwnedPacket<PacketType>(
    		OwnedPacketTCP<PacketType>{this->shared_from_this(), this->incoming_packet_}));
            read_header();
          } else {
            std::cerr << "[ServerTCP] Error reading body: " << ec.message() << std::endl;
            this->disconnect();
          }
        });
  }

 private:
  ConcurrentQueue<OwnedPacket<PacketType>>& received_queue_;

  uint32_t connection_id_;
};

}  // namespace network

#endif  // SERVER_TCP_CONNECTION_HPP_
