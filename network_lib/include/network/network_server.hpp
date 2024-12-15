#ifndef NETWORK_SERVER_H_
#define NETWORK_SERVER_H_

#include <__algorithm/ranges_find_if.h>

#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <cstdint>
#include <deque>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include "concurrent_queue.hpp"
#include "tcp_server_connection.hpp"
#include "udp_connection.hpp"

namespace network {

template <typename PacketType>
class NetworkServer {
 public:
  explicit NetworkServer(const uint16_t tcp_port, const uint16_t udp_port)
    : acceptor_(io_context_,
                  asio::ip::tcp::endpoint(asio::ip::tcp::v4(), tcp_port)),
    udp_connection_(std::make_shared<UdpConnection<PacketType>>(io_context_, udp_port, received_queue_)) {}

  virtual ~NetworkServer() { stop(); }

  bool start() {
    try {
      accept();
      udp_connection_->start();
      context_thread_ = std::thread([this]() { io_context_.run(); });
      std::cout << "[Server][INFO] Started successfully on port "
                << acceptor_.local_endpoint().port() << std::endl;
      return true;
    } catch (const std::exception& e) {
      std::cerr << "[Server][ERROR] Exception during start: " << e.what()
                << std::endl;
      return false;
    }
  }

  void stop() {
    io_context_.stop();

    if (context_thread_.joinable()) {
      context_thread_.join();
    }
    connections_.clear();
    udp_to_tcp_map_.clear();
    tcp_to_udp_map_.clear();
    std::cout << "[Server][INFO] Stopped." << std::endl;
  }

  template <typename T>
  void broadcast_tcp(T&& packet) {
    std::erase_if(connections_, [this, &packet](auto& connection) {
        if (connection->is_connected()) {
            std::cout << "[Server][INFO] Sending packet " << packet << std::endl;
            connection->send(std::forward<T>(packet));
            return false;
        }
        on_client_disconnect(connection);
        return true;
    });
  }

  template <typename T>
  void broadcast_udp(const Packet<PacketType>& packet) {
    for (const auto& [udp_endpoint, connection] : udp_to_tcp_map_) {
      udp_connection_->send_to(packet, udp_endpoint);
    }
  }

  template <typename T>
  void broadcast_to_others_tcp(
    const std::shared_ptr<TcpServerConnection<PacketType>>& excluded_connection,
    T&& packet) {
    std::erase_if(connections_, [this, &packet, &excluded_connection](auto& connection) {
        if (connection == excluded_connection) {
            return false;
        }
        if (connection->is_connected()) {
            std::cout << "[Server][INFO] Broadcasting packet to client ID "
                      << connection->get_id() << std::endl;
            connection->send(std::forward<T>(packet));
            return false;
        }
        on_client_disconnect(connection);
        return true;
    });
  }

  template <typename T>
  bool send_to_tcp(uint32_t connection_id, T&& packet) {
    bool sent = false;

    std::erase_if(connections_,
                  [this, &packet, connection_id, &sent](auto& connection) {
                    if (connection->get_id() == connection_id) {
                      if (connection->is_connected()) {
                        std::cout << "[Server][INFO] Sending packet " << packet << std::endl;
                        connection->send(std::forward<T>(packet));
                        sent = true;
                        return false;
                      }
                      on_client_disconnect(connection);
                      return true;
                    }
                    return false;
                  });

    if (!sent) {
      std::cerr << "[Server][ERROR] Failed to send packet to client ID "
                << connection_id << std::endl;
    }
    return sent;
  }

  template <typename T>
  bool send_to_tcp(const std::shared_ptr<TcpServerConnection<PacketType>>& connection, T&& packet) {
    if (connection && connection->is_connected()) {
      connection->send(std::forward<T>(packet));
      return true;
    }

    std::cerr << "[Server][INFO] Removing invalid or disconnected connection ID "
      << (connection ? connection->get_id() : 0) << std::endl;

    on_client_disconnect(connection);
    connections_.erase(std::remove(connections_.begin(), connections_.end(), connection), connections_.end());

    return false;
  }

  template <typename T>
  void send_to_udp(const asio::ip::udp::endpoint& endpoint, const T& packet) {
    udp_connection_->send_to(packet, endpoint);
  }

  bool disconnect_client(uint32_t connection_id) {
    const auto it = std::ranges::find_if(connections_, [connection_id](const auto& conn) {
      return conn && conn->get_id() == connection_id;
    });

    if (it != connections_.end()) {
      auto connection = *it;

      auto udp_it = tcp_to_udp_map_.find(connection);
      if (udp_it != tcp_to_udp_map_.end()) {
        udp_to_tcp_map_.erase(udp_it->second);
        tcp_to_udp_map_.erase(udp_it);
      }

      on_client_disconnect(connection);
      connection->disconnect();
      connections_.erase(it);

      std::cout << "[Server][INFO] Client with ID " << connection_id
                << " disconnected successfully." << std::endl;
      return true;
    }

    std::cerr << "[Server][ERROR] Failed to disconnect client ID "
              << connection_id << std::endl;
    return false;
  }


  virtual void register_udp_endpoint(const std::shared_ptr<TcpServerConnection<PacketType>>& connection, uint16_t udp_port) {
    auto client_ip = connection->get_socket().remote_endpoint().address();
    asio::ip::udp::endpoint udp_endpoint(client_ip, udp_port);

    udp_to_tcp_map_[udp_endpoint] = connection;
    tcp_to_udp_map_[connection] = udp_endpoint;

    std::cout << "[Server][INFO] Registered UDP endpoint for client ID "
              << connection->get_id() << " at " << udp_endpoint.address().to_string()
              << ":" << udp_endpoint.port() << std::endl;
  }

  std::optional<OwnedPacket<PacketType>> pop_message() { return received_queue_.pop(); }

  [[nodiscard]] size_t get_connection_count() const {
    return connections_.size();
  }

 protected:
  /**
   * Method to handle client connection logic.
   * @param connection The new connection object.
   * @return `true` if the connection is accepted, `false` otherwise.
   */
  virtual bool on_client_connect(
      const std::shared_ptr<TcpServerConnection<PacketType>>& connection) {
    // Default implementation: always accept connections.
    // Override this in a derived class to implement custom logic.
    return true;
  }

  /**
  * Callback when a client is fully accepted (customizable in derived classes).
  */
  virtual void on_client_accepted(
      const std::shared_ptr<TcpServerConnection<PacketType>>& connection) {
    // Default: Do nothing. Override in derived classes.
  }

  virtual void on_client_disconnect(
      const std::shared_ptr<TcpServerConnection<PacketType>>& connection) {

      std::cout << "[Server][INFO] Client with ID " << connection->get_id()
                << " disconnected." << std::endl;
  }

 private:
  void accept() {
    acceptor_.async_accept(
        [this](const std::error_code& ec, asio::ip::tcp::socket socket) {
          if (!ec) {
            std::cout << "[Server][INFO] New connection attempt from "
                      << socket.remote_endpoint().address().to_string()
                      << std::endl;

            auto connection = std::make_shared<TcpServerConnection<PacketType>>(std::move(socket), received_queue_,
                ++connection_id_counter_);

            if (on_client_connect(connection)) {
              connections_.emplace_back(connection);
              std::cout << "[Server][INFO] Connection accepted with ID "
                        << connection->get_id() << std::endl;
              connection->start();
              on_client_accepted(connection);
            } else {
              std::cout << "[Server][INFO] Connection rejected from "
                        << socket.remote_endpoint().address().to_string()
                        << std::endl;
              socket.close();
            }
          } else {
            std::cerr << "[Server][ERROR] Accept error: " << ec.message()
                      << std::endl;
          }

          accept();
        });
  }

  void disconnect_all_clients() {
    for (const auto& connection : connections_) {
      if (connection && connection->is_connected()) {
        connection->disconnect();
      }
    }
    connections_.clear();
  }

  asio::io_context io_context_{};
  asio::ip::tcp::acceptor acceptor_;

  std::shared_ptr<UdpConnection<PacketType>> udp_connection_;

  ConcurrentQueue<OwnedPacket<PacketType>> received_queue_;
  std::deque<std::shared_ptr<TcpServerConnection<PacketType>>> connections_;

  std::unordered_map<asio::ip::udp::endpoint, std::shared_ptr<TcpServerConnection<PacketType>>> udp_to_tcp_map_;
  std::unordered_map<std::shared_ptr<TcpServerConnection<PacketType>>, asio::ip::udp::endpoint> tcp_to_udp_map_;

  std::thread context_thread_;

  uint32_t connection_id_counter_ = 0;
};
}  // namespace network

#endif //NETWORK_SERVER_H_
