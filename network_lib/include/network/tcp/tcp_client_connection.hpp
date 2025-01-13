#ifndef NETWORK_TCP_CLIENT_CONNECTION_HPP_
#define NETWORK_TCP_CLIENT_CONNECTION_HPP_

#include <asio.hpp>
#include <iostream>
#include <memory>
#include <utility>

#include "tcp_connection.hpp"

namespace network {

/**
 * @brief TCP Client Connection class.
 *
 * This class handles client-specific TCP connection logic, including establishing
 * a connection to a server and processing received packets.
 *
 * @tparam PacketType The type of packet used in the connection.
 */
template <typename PacketType>
class TcpClientConnection final : public TcpConnection<PacketType>,
                                  public std::enable_shared_from_this<TcpClientConnection<PacketType>> {
 public:
  /**
   * @brief Constructs a TcpClientConnection.
   *
   * Initializes the connection with the provided socket and a shared queue
   * for storing received packets.
   *
   * @param socket The TCP socket for the connection.
   * @param received_queue The queue where received packets will be pushed.
   */
  explicit TcpClientConnection(asio::ip::tcp::socket socket,
                                ConcurrentQueue<Packet<PacketType>>& received_queue)
      : TcpConnection<PacketType>(std::move(socket)), received_queue_(received_queue) {}

  /**
   * @brief Connects to a server using the specified endpoints.
   *
   * This method initiates an asynchronous connection attempt to the provided server endpoints.
   * Upon completion, the `connection_result` promise is set to indicate success or failure.
   *
   * @param endpoints The resolver results containing the server endpoints.
   * @param connection_result A promise to store the connection result (`true` for success, `false` for failure).
   */
  void Connect(const asio::ip::tcp::resolver::results_type& endpoints,
               std::promise<bool>& connection_result) {
    auto self = this->shared_from_this();
    asio::async_connect(
        this->socket_, endpoints,
        [self, &connection_result](const std::error_code& ec, const asio::ip::tcp::endpoint&) {
          if (!ec) {
            connection_result.set_value(true);
            self->ReadHeader(self);
          } else {
            connection_result.set_value(false);
            std::cerr << "[TCP Client] Connection error: " << ec.message() << std::endl;
            self->Disconnect();
          }
        });
  }

 protected:
  /**
   * @brief Returns a shared pointer to the current instance.
   *
   * Used internally to maintain the lifetime of the connection object
   * during asynchronous operations.
   *
   * @return A shared pointer to this `TcpClientConnection` instance.
   */
  std::shared_ptr<TcpConnection<PacketType>> ThisShared() override {
    return this->shared_from_this();
  }

  /**
   * @brief Callback invoked when a fully received packet is ready.
   *
   * Pushes the packet into the `received_queue_` for further processing.
   *
   * @param self A shared pointer to this connection instance.
   * @param packet The fully received packet.
   */
  void OnPacketReceived(Packet<PacketType>&& packet) override {
    received_queue_.Push(std::move(packet));
  }

 private:
  /**
   * @brief Queue to store received packets.
   *
   * Shared between the client and its handler, this queue stores packets
   * received from the server for further processing.
   */
  ConcurrentQueue<Packet<PacketType>>& received_queue_;
};

}  // namespace network

#endif  // NETWORK_TCP_CLIENT_CONNECTION_HPP_
