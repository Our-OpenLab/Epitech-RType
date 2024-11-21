#ifndef CLIENT_HPP_
#define CLIENT_HPP_

#include <thread>
#include <asio/io_context.hpp>
#include <memory>
#include <mutex>
#include <future>
#include <sstream>

#include "concurrent_queue.hpp"
#include "protocol.hpp"
#include "client_connection.hpp"

namespace network {
class Client {
public:
  Client() : io_context_(), received_queue_() {}

  ~Client() {}

  bool connect(const std::string& host, const std::string& port) {
    try {
      std::cout << "[Client][INFO] Attempting to connect to " << host << ":" << port << "..." << std::endl;

      asio::ip::tcp::resolver resolver(io_context_);
      const auto endpoints = resolver.resolve(host, port);

      connection_ = std::make_unique<ClientConnection>(io_context_, asio::ip::tcp::socket(io_context_), received_queue_);

      std::promise<bool> connection_promise;
      auto connection_future = connection_promise.get_future();

      connection_->connect(endpoints, connection_promise);

      context_thread_ = std::thread([this]() { io_context_.run(); });

      if (const bool is_connected = connection_future.get(); !is_connected) {
        std::cerr << "[Client][ERROR] Failed to connect to " << host << ":" << port << std::endl;
        io_context_.stop();
        if (context_thread_.joinable()) {
          context_thread_.join();
        }
        return false;
      }

      return true;
    } catch (const std::exception& e) {
      std::cerr << "[Client][ERROR] Connection exception: " << e.what() << std::endl;

      io_context_.stop();
      if (context_thread_.joinable()) {
        context_thread_.join();
      }

      return false;
    }
  };

  void disconnect() {
    if (is_connected()) {
      std::cout << "[Client][INFO] Disconnecting from the server..." << std::endl;
      connection_->disconnect();
    }

    io_context_.stop();
    if (context_thread_.joinable()) {
      context_thread_.join();
    }
    connection_.reset();
    std::cout << "[Client][INFO] Client disconnected successfully." << std::endl;
  }

  bool is_connected() const { return connection_ && connection_->is_connected(); }

  template <typename T>
  void send(T&& packet) {
    if (!is_connected()) {
      std::cerr << "[Client][ERROR] Attempt to send a packet failed: connection is not active." << std::endl;
      return;
    }
    connection_->send(std::forward<T>(packet));
    std::cout << "[Client][INFO] Packet sent to the server." << std::endl;
  }

  ConcurrentQueue<Packet>& get_received_queue() { return received_queue_; }

  private:

  asio::io_context io_context_;
  ConcurrentQueue<Packet> received_queue_;
  std::unique_ptr<ClientConnection> connection_;
  std::thread context_thread_;
};
}

#endif
