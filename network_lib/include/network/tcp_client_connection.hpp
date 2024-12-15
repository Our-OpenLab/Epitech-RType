#ifndef CLIENT_TCP_CONNECTION_HPP_
#define CLIENT_TCP_CONNECTION_HPP_

#include "tcp_connection.hpp"

namespace network {
template <typename PacketType>
class ClientTCPConnection final: public TcpConnection<PacketType>, public std::enable_shared_from_this<ClientTCPConnection<PacketType>> {
 public:
  ClientTCPConnection(asio::ip::tcp::socket socket,
                   ConcurrentQueue<Packet<PacketType>>& received_queue)
      : TcpConnection<PacketType>(std::move(socket)), received_queue_(received_queue) {}

  void connect(const asio::ip::tcp::resolver::results_type& endpoints,
               std::promise<bool>& connection_result) {
    auto self = this->shared_from_this();
    asio::async_connect(
        self->socket_, endpoints,
        [self, &connection_result](
            const std::error_code& ec,
            const asio::ip::tcp::endpoint& /*endpoint*/) mutable {
          if (!ec) {
            connection_result.set_value(true);
            self->read_header();
          } else {
            connection_result.set_value(false);
            std::cerr << "[ClientTCP] Error reading header: " << ec.message() << std::endl;
            self->disconnect();
          }
        });
  }

 protected:

  std::shared_ptr<TcpConnection<PacketType>> this_shared() override {
    return this->shared_from_this();
  }

  void read_header() override {
    asio::async_read(
        this->socket_, asio::buffer(&this->incoming_packet_.header, sizeof(Header<PacketType>)),
        [this](const std::error_code& ec, std::size_t /*length*/) {
          if (!ec) {
            if (this->incoming_packet_.header.size > 0) {
              if (this->incoming_packet_.header.size > this->MAX_BODY_SIZE) {
                std::cerr << "[ClientTCP] Error reading header: Invalid Body Size" << std::endl;
            	this->disconnect();
                return;
              }
              this->incoming_packet_.body.resize(this->incoming_packet_.header.size);
              read_body();
            } else {
              received_queue_.push(this->incoming_packet_);
              read_header();
            }
          } else {
            std::cerr << "[ClientTCP] Error reading header: " << ec.message() << std::endl;
            this->disconnect();
          }
        });
  }

  void read_body() override {
    asio::async_read(this->socket_,
                     asio::buffer(this->incoming_packet_.body.data(),
                                  this->incoming_packet_.header.size),
                     [this](const std::error_code& ec, std::size_t /*length*/) {
                       if (!ec) {
                         received_queue_.push(this->incoming_packet_);
                         read_header();
                       } else {
                         std::cerr << "[ClientTCP] Error reading body: " << ec.message() << std::endl;
                         this->disconnect();
                       }
                     });
  }

 private:

  ConcurrentQueue<Packet<PacketType>>& received_queue_;
};
}  // namespace network

#endif  // CLIENT_TCP_CONNECTION_HPP_
