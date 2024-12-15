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
#include "network/tcp/tcp_client_connection.hpp"
#include "network/udp/udp_client_connection.hpp"

namespace network {

/**
 * @brief Network client class for managing TCP and UDP connections.
 *
 * This class handles both TCP and UDP connections, allowing bidirectional
 * communication with a server.
 *
 * @tparam PacketType The type of packet used in the connection.
 */
template <typename PacketType>
class NetworkClient {
 public:
  /**
   * @brief Constructs a NetworkClient and establishes connections.
   *
   * @param host The server hostname or IP address.
   * @param service The server TCP service or port.
   * @param udp_port The UDP port for communication.
   * @throws std::runtime_error If the connection fails.
   */
  NetworkClient(const std::string& host, const std::string& service, uint16_t udp_port) {
    try {
      std::cout << "[Client][INFO] Connecting to " << host << ":" << service << "..." << std::endl;

      // Resolve TCP connection endpoints
      asio::ip::tcp::resolver tcp_resolver(io_context_);
      const auto tcp_endpoints = tcp_resolver.resolve(host, service);

      tcp_connection_ = std::make_shared<TcpClientConnection<PacketType>>(
          asio::ip::tcp::socket(io_context_), received_queue_);

      std::promise<bool> connection_promise;
      auto connection_future = connection_promise.get_future();

      tcp_connection_->Connect(tcp_endpoints, connection_promise);

      // Start the ASIO context thread
      context_thread_ = std::thread([this]() { io_context_.run(); });

      if (!connection_future.get()) {
        std::cerr << "[Client][ERROR] Failed to connect to " << host << ":" << service << std::endl;
        throw std::runtime_error("TCP connection failed");
      }

      std::cout << "[Client][INFO] Successfully connected to " << host << ":" << service << std::endl;

      // Resolve UDP connection endpoint
      asio::ip::udp::resolver udp_resolver(io_context_);
      const auto udp_endpoints = udp_resolver.resolve(host, std::to_string(udp_port));

      if (udp_endpoints.empty()) {
        throw std::runtime_error("Failed to resolve UDP host: " + host);
      }

      server_udp_endpoint_ = *udp_endpoints.begin();

      udp_connection_ = std::make_shared<UdpClientConnection<PacketType>>(io_context_, received_queue_);
      udp_connection_->Start();

      std::cout << "[Client][INFO] UDP connection to " << host << ":" << udp_port << " initialized." << std::endl;

    } catch (const std::exception& e) {
      std::cerr << "[Client][ERROR] Exception during connection: " << e.what() << std::endl;
      Disconnect();
      throw;
    }
  }

  /**
   * @brief Destructor for the NetworkClient.
   *
   * Ensures proper cleanup of resources.
   */
  ~NetworkClient() { Disconnect(); }

  /**
   * @brief Disconnects the client from the server.
   */
  void Disconnect() {
    if (udp_connection_) {
      udp_connection_->Close();
    }

    io_context_.stop();

    if (context_thread_.joinable()) {
      context_thread_.join();
    }

    tcp_connection_.reset();
    udp_connection_.reset();

    std::cout << "[Client][INFO] Client disconnected successfully." << std::endl;
  }

  /**
   * @brief Checks if the TCP connection is active.
   *
   * @return `true` if connected, `false` otherwise.
   */
  [[nodiscard]] bool IsTcpConnected() const {
    return tcp_connection_ && tcp_connection_->IsConnected();
  }

  /**
   * @brief Checks if the UDP connection is active.
   *
   * @return `true` if the UDP connection is open, `false` otherwise.
   */
  [[nodiscard]] bool IsUdpConnected() const {
    return udp_connection_ && udp_connection_->IsOpen();
  }

  /**
   * @brief Sends a packet over the TCP connection.
   *
   * @tparam T The type of the packet to send.
   * @param packet The packet to send.
   */
  template <typename T>
  void SendTcp(T&& packet) const {
    if (!IsTcpConnected()) {
      std::cerr << "[Client][ERROR] Attempt to send TCP packet failed: connection is not active." << std::endl;
      return;
    }
    tcp_connection_->Send(std::forward<T>(packet));
  }

  /**
   * @brief Sends a packet over the UDP connection.
   *
   * @tparam T The type of the packet to send.
   * @param packet The packet to send.
   */
  template <typename T>
  void SendUdp(T&& packet) const {
    if (!IsUdpConnected()) {
      std::cerr << "[Client][UDP][ERROR] Attempt to send UDP packet failed: connection is not active." << std::endl;
      return;
    }
    udp_connection_->SendTo(std::forward<T>(packet), server_udp_endpoint_);
  }

  /**
   * @brief Retrieves the next received message.
   *
   * @return An optional Packet object.
   */
  std::optional<Packet<PacketType>> PopMessage() { return received_queue_.Pop(); }

  /**
   * @brief Gets the local UDP port used by the client.
   *
   * @return The local UDP port.
   */
  [[nodiscard]] uint16_t GetLocalUdpPort() const {
    return udp_connection_->GetLocalPort();
  }

 private:
  asio::io_context io_context_;  ///< ASIO I/O context for handling asynchronous operations.
  asio::ip::udp::endpoint server_udp_endpoint_;  ///< Server's UDP endpoint.
  ConcurrentQueue<Packet<PacketType>> received_queue_;  ///< Queue for received packets.
  std::shared_ptr<TcpClientConnection<PacketType>> tcp_connection_;  ///< TCP connection instance.
  std::shared_ptr<UdpClientConnection<PacketType>> udp_connection_;  ///< UDP connection instance.
  std::thread context_thread_;  ///< Thread running the ASIO context.
};

}  // namespace network

#endif  // NETWORK_CLIENT_HPP_
