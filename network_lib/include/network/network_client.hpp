#ifndef NETWORK_CLIENT_HPP_
#define NETWORK_CLIENT_HPP_

#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <future>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>

#include "concurrent_queue.hpp"
#include "protocol.hpp"
#include "tcp_client_connection.hpp"

namespace network {
template <typename PacketType>
class NetworkClient {
public:
  NetworkClient(const std::string& host, const std::string& service)
      : connection_(nullptr) {
    try {
      std::cout << "[Client][INFO] Attempting to connect to " << host << ":" << service << "..." << std::endl;

      asio::ip::tcp::resolver resolver(io_context_);
      const auto endpoints = resolver.resolve(host, service);

      connection_ = std::make_shared<ClientTCPConnection<PacketType>>(asio::ip::tcp::socket(io_context_), received_queue_);

      std::promise<bool> connection_promise;
      auto connection_future = connection_promise.get_future();

      connection_->connect(endpoints, connection_promise);

      context_thread_ = std::thread([this]() { io_context_.run(); });

      if (const bool is_connected = connection_future.get(); !is_connected) {
        std::cerr << "[Client][ERROR] Failed to connect to " << host << ":" << service << std::endl;
        throw std::runtime_error("Connection failed");
      }

      std::cout << "[Client][INFO] Successfully connected to " << host << ":" << service << std::endl;

    } catch (const std::exception& e) {
      std::cerr << "[Client][ERROR] Connection exception: " << e.what() << std::endl;

      io_context_.stop();
      if (context_thread_.joinable()) {
        context_thread_.join();
      }

      throw;
    }
  }

  ~NetworkClient() {
    disconnect();
  }

  /*

  bool connect(const std::string& host, const std::string& service) {
    try {
      std::cout << "[Clienvt][INFO] Attempting to connect to " << host << ":" << service << "..." << std::endl;

      asio::ip::tcp::resolver resolver(io_context_);
      const auto endpoints = resolver.resolve(host, service);

      connection_ = std::make_unique<ClientConnection>(io_context_, asio::ip::tcp::socket(io_context_), received_queue_);

      std::promise<bool> connection_promise;
      auto connection_future = connection_promise.get_future();

      connection_->connect(endpoints, connection_promise);

      context_thread_ = std::thread([this]() { io_context_.run(); });

      if (const bool is_connected = connection_future.get(); !is_connected) {
        std::cerr << "[Client][ERROR] Failed to connect to " << host << ":" << service << std::endl;
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
  */

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

  [[nodiscard]] bool is_connected() const { return connection_ && connection_->is_connected(); }

  template <typename T>
  void send(T&& packet) {
    if (!is_connected()) {
      std::cerr << "[Client][ERROR] Attempt to send a packet failed: connection is not active." << std::endl;
      return;
    }
    std::cout << "[Client][INFO] Sending packet " << packet << std::endl;
    connection_->send(std::forward<T>(packet));
  }

  std::optional<Packet<PacketType>> pop_message() { return received_queue_.pop(); }

  private:

  asio::io_context io_context_;
  ConcurrentQueue<Packet<PacketType>> received_queue_;
  std::shared_ptr<ClientTCPConnection<PacketType>> connection_;
  std::thread context_thread_;
};
}

#endif
