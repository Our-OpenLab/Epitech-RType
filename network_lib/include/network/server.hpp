#ifndef SERVER_H_
#define SERVER_H_

#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <cstdint>
#include <deque>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include "concurrent_queue.hpp"
#include "protocol.hpp"
#include "server_connection.hpp"

namespace network {
class Server {
 public:
  explicit Server(const uint16_t port)
      : acceptor_(io_context_,
                  asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {}

  virtual ~Server() { stop(); }

  void start() {
    try {
      accept();
      context_thread_ = std::thread([this]() { io_context_.run(); });
      std::cout << "[Server][INFO] Started successfully on port "
                << acceptor_.local_endpoint().port() << std::endl;
    } catch (const std::exception& e) {
      std::cerr << "[Server][ERROR] Exception during start: " << e.what()
                << std::endl;
    }
  }

  void stop() {
    io_context_.stop();

    if (context_thread_.joinable()) {
      context_thread_.join();
    }
    connections_.clear();
    std::cout << "[Server][INFO] Stopped." << std::endl;
  }

  template <typename T>
  void broadcast(T&& packet) {
    // std::lock_guard<std::mutex> lock(connections_mutex_);
    std::erase_if(connections_, [this, &packet](const std::shared_ptr<ServerConnection>& connection) {
        if (connection && connection->is_connected()) {
            connection->send(std::forward<T>(packet));
            return false;
        }
        on_client_disconnect(connection);
        return true;
    });
  }

  template <typename T>
  bool send_to(uint32_t connection_id, T&& packet) {
    // std::lock_guard<std::mutex> lock(connections_mutex_);
    bool sent = false;

    std::erase_if(connections_,
                  [this, &packet, connection_id, &sent](const auto& conn) {
                    if (conn && conn->get_id() == connection_id) {
                      if (conn->is_connected()) {
                        conn->send(std::forward<T>(packet));
                        sent = true;
                        return false;
                      } else {
                        on_client_disconnect(conn);
                        return true;
                      }
                    }
                    return false;
                  });

    if (!sent) {
      std::cerr << "[Server][ERROR] Failed to send packet to client ID "
                << connection_id << std::endl;
    }
    return sent;
  }

  bool disconnect_client(uint32_t connection_id) {
    // std::lock_guard<std::mutex> lock(connections_mutex_);
    const auto it =
        std::ranges::find_if(connections_, [connection_id](const auto& conn) {
          return conn && conn->get_id() == connection_id;
        });

    if (it != connections_.end()) {
      on_client_disconnect(*it);
      (*it)->disconnect();
      connections_.erase(it);
      std::cout << "[Server][INFO] Client with ID " << connection_id
                << " disconnected successfully." << std::endl;
      return true;
    }
    std::cerr << "[Server][ERROR] Failed to disconnect client ID "
              << connection_id << std::endl;
    return false;
  }

  [[nodiscard]] size_t get_connection_count() const {
    // std::lock_guard<std::mutex> lock(connections_mutex_);
    return connections_.size();
  }

 protected:
  /**
   * Method to handle client connection logic.
   * @param connection The new connection object.
   * @return `true` if the connection is accepted, `false` otherwise.
   */
  virtual bool on_client_connect(
      const std::shared_ptr<ServerConnection>& connection) {
    // Default implementation: always accept connections.
    // Override this in a derived class to implement custom logic.
    return true;
  }

  virtual void on_client_disconnect(
      const std::shared_ptr<ServerConnection>& connection) {
    if (connection) {
      std::cout << "[Server][INFO] Client with ID " << connection->get_id()
                << " disconnected." << std::endl;
    }
  }

 private:
  void accept() {
    acceptor_.async_accept(
        [this](const std::error_code& ec, asio::ip::tcp::socket socket) {
          if (!ec) {
            std::cout << "[Server][INFO] New connection attempt from "
                      << socket.remote_endpoint().address().to_string()
                      << std::endl;

            auto connection = std::make_shared<ServerConnection>(
                io_context_, std::move(socket), received_queue_,
                ++connection_id_counter_);

            if (on_client_connect(connection)) {
              // std::lock_guard<std::mutex> lock(connections_mutex_);
              connections_.emplace_back(connection);
              std::cout << "[Server][INFO] Connection accepted with ID "
                        << connection->get_id() << std::endl;
              connection->start();
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
    // std::lock_guard<std::mutex> lock(connections_mutex_);
    for (const auto& connection : connections_) {
      if (connection && connection->is_connected()) {
        connection->disconnect();
      }
    }
    connections_.clear();
  }

  ConcurrentQueue<OwnedPacket> received_queue_;

  std::deque<std::shared_ptr<ServerConnection>> connections_;

  asio::io_context io_context_;
  std::thread context_thread_;
  asio::ip::tcp::acceptor acceptor_;

  // mutable std::mutex connections_mutex_;

  uint32_t connection_id_counter_ = 0;
};
}  // namespace network

#endif
