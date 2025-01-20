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
 * communication with a server. It uses an internal `asio::io_context` and
 * a dedicated thread to run it. Received packets are stored in a concurrent
 * queue for later retrieval via `PopMessage()`.
 *
 * @tparam PacketType The type of packet used in the connection.
 */
template <typename PacketType>
class NetworkClient {
 public:
  /**
  * @brief Default constructor. No connection is established at this point.
  *
  * Call `Connect()` to actually connect the client.
  */
  NetworkClient() = default;

  /**
   * @brief Destructor for the NetworkClient.
   *
   * Ensures proper cleanup of resources by calling `Disconnect()`.
   */
  ~NetworkClient() {
    Disconnect();
  }

  /**
   * @brief Initiates the TCP and UDP connections to the server.
   *
   * If a connection already exists, it will be shut down first.
   *
   * @param host     The server hostname or IP address.
   * @param service  The server TCP service or port (e.g., "80", "443").
   * @param udp_port The UDP port for communication.
   * @return true if the connection was successfully established, false otherwise.
   */
  bool Connect(const std::string& host, const std::string& service, const uint16_t udp_port) {
    Disconnect();

    try {
      std::cout << "[Client][INFO] Connecting to " << host << ":" << service << "..." << std::endl;

      asio::ip::tcp::resolver tcp_resolver(io_context_);
      auto tcp_endpoints = tcp_resolver.resolve(host, service);

      tcp_connection_ = std::make_shared<TcpClientConnection<PacketType>>(
          asio::ip::tcp::socket(io_context_), received_queue_);

      std::promise<bool> connection_promise;
      auto connection_future = connection_promise.get_future();

      tcp_connection_->Connect(tcp_endpoints, connection_promise);

      context_thread_ = std::thread([this]() {
        io_context_.run();
      });

      if (!connection_future.get()) {
        std::cerr << "[Client][ERROR] Failed to connect to " << host << ":" << service << std::endl;
        Disconnect();
        return false;
      }

      std::cout << "[Client][INFO] Successfully connected to " << host << ":" << service << std::endl;
      connected_host_ = host;

      asio::ip::udp::resolver udp_resolver(io_context_);
      const auto udp_endpoints = udp_resolver.resolve(host, std::to_string(udp_port));

      if (udp_endpoints.empty()) {
        std::cerr << "[Client][ERROR] Failed to resolve UDP host: " << host << std::endl;
        Disconnect();
        return false;
      }

      server_udp_endpoint_ = *udp_endpoints.begin();

      udp_connection_ = std::make_shared<UdpClientConnection<PacketType>>(io_context_, received_queue_);
      udp_connection_->Start();

      std::cout << "[Client][INFO] UDP connection to " << host << ":" << udp_port << " initialized." << std::endl;

    } catch (const std::exception& e) {
      std::cerr << "[Client][ERROR] Exception during connection: " << e.what() << std::endl;
      Disconnect();
      return false;
    }

    return true;
  }

  /**
   * @brief Disconnects the client from the server.
   *
   * Stops the I/O context, joins the context thread, and resets the TCP/UDP connections.
   */
  void Disconnect() {
    if (!IsTcpConnected() && !IsUdpConnected()) {
      return;
    }

    if (udp_connection_) {
      udp_connection_->Close();
    }

    if (tcp_connection_) {
      tcp_connection_->Disconnect();
    }

    io_context_.stop();

    if (context_thread_.joinable()) {
      context_thread_.join();
    }

    tcp_connection_.reset();
    udp_connection_.reset();

    io_context_.reset();
    connected_host_.clear();
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
   * @param packet The packet to send (universal reference).
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
   * @param packet The packet to send (universal reference).
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
   * @brief Retrieves the next received message from the internal queue.
   *
   * @return An optional Packet object if any is available, or std::nullopt otherwise.
   */
  std::optional<Packet<PacketType>> PopMessage() { return received_queue_.Pop(); }

  /**
   * @brief Gets the local UDP port used by the client.
   *
   * @return The local UDP port. Returns 0 if UDP is not connected.
   */
  [[nodiscard]] uint16_t GetLocalUdpPort() const {
    if (udp_connection_) {
      return udp_connection_->GetLocalPort();
    }
    return 0;
  }

  /**
   * @brief Gets a reference to the internal ASIO I/O context.
   *
   * This method exposes the internal `asio::io_context` instance, which can
   * be used for additional asynchronous operations or integration with
   * external components.
   *
   * @return A reference to the internal `asio::io_context`.
   */
  [[nodiscard]] asio::io_context& GetIoContext() {
    return io_context_;
  }

  /**
   * @brief Gets the host to which the client is connected.
   *
   * @return The host as a string, or an empty string if not connected.
   */
  [[nodiscard]] const std::string &GetHost() const {
    return connected_host_;
  }

 private:
  asio::io_context io_context_;  ///< ASIO I/O context for handling asynchronous operations.
  asio::ip::udp::endpoint server_udp_endpoint_;  ///< Server's UDP endpoint.
  ConcurrentQueue<Packet<PacketType>> received_queue_;  ///< Queue for received packets.
  std::shared_ptr<TcpClientConnection<PacketType>> tcp_connection_;  ///< TCP connection instance.
  std::shared_ptr<UdpClientConnection<PacketType>> udp_connection_;  ///< UDP connection instance.
  std::thread context_thread_;  ///< Thread running the ASIO context.

  std::string connected_host_;  ///< Hostname or IP address of the connected server.
};

}  // namespace network

#endif  // NETWORK_CLIENT_HPP_
