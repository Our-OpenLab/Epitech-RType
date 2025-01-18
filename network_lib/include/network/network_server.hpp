#ifndef NETWORK_SERVER_H_
#define NETWORK_SERVER_H_

#include <algorithm>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <cstdint>
#include <deque>
#include <iostream>
#include <memory>
#include <thread>
#include <unordered_map>
#include <vector>

#include "concurrent_queue.hpp"
#include "network/tcp/tcp_server_connection.hpp"
#include "network/udp/udp_server_connection.hpp"

namespace network {

/**
 * @brief A network server managing both TCP and UDP connections.
 *
 * This class handles:
 * - Accepting incoming TCP clients (asynchronously)
 * - Starting and managing UDP connections
 * - Broadcasting and sending data to TCP/UDP clients
 * - Maintaining a periodic timer to check and remove disconnected clients
 *
 * @tparam PacketType The type of packets used in the server.
 */
template <typename PacketType>
class NetworkServer
    : public std::enable_shared_from_this<NetworkServer<PacketType>> {
 public:
  /**
   * @brief Constructs the network server, creating the ASIO acceptor and UDP
   * connection.
   * @param tcp_port The TCP port on which to accept incoming connections.
   * @param udp_port The UDP port for receiving/sending datagrams.
   * @param check_interval_ms Interval (in milliseconds) to periodically check
   * for disconnected clients.
   */
  explicit NetworkServer(const uint16_t tcp_port, const uint16_t udp_port,
                         const uint64_t check_interval_ms = 5000)
      : acceptor_(io_context_,
                  asio::ip::tcp::endpoint(asio::ip::tcp::v4(), tcp_port)),
        udp_connection_(std::make_shared<UdpServerConnection<PacketType>>(io_context_, udp_port, received_queue_)),
        check_timer_(io_context_),
        check_interval_ms_(check_interval_ms) {}

  /**
   * @brief Destructor. Calls Stop() to ensure resources are freed.
   */
  virtual ~NetworkServer() { Stop(); }

  /**
   * @brief Starts the server:
   *  1) Accepts new TCP connections asynchronously,
   *  2) Starts the UDP connection,
   *  3) Schedules a periodic timer to check connections,
   *  4) Runs the I/O context in a dedicated thread.
   *
   * @return true if the server started successfully, false otherwise.
   */
  bool Start() {
    try {
      auto self = this->shared_from_this();

      Accept(self);

      if (udp_connection_) {
        udp_connection_->Start();
      }

      StartCheckTimer(self);

      context_thread_ = std::thread([this]() {
        io_context_.run();
      });

      std::cout << "[Server][INFO] Started successfully on TCP port "
                << acceptor_.local_endpoint().port() << std::endl;
      return true;
    } catch (const std::exception& e) {
      std::cerr << "[Server][ERROR] Failed to start: " << e.what() << std::endl;
      return false;
    }
  }

  /**
   * @brief Stops the server by:
   *  1) Disconnecting all TCP connections,
   *  2) Closing the UDP connection,
   *  3) Stopping the I/O context,
   *  4) Clearing internal maps.
   */
  void Stop() {
    try {
      DisconnectAllClients();

      if (udp_connection_) {
        udp_connection_->Close();
      }

      io_context_.stop();

      if (context_thread_.joinable()) {
        context_thread_.join();
      }

      udp_to_tcp_map_.clear();
      tcp_to_udp_map_.clear();

      std::cout << "[Server][INFO] Stopped." << std::endl;
    } catch (const std::exception& e) {
      std::cerr << "[Server][ERROR] Failed to stop: " << e.what() << std::endl;
    }
  }

  /**
   * @brief Broadcasts a packet to all active TCP connections.
   *        Disconnected clients are removed on the fly.
   * @tparam T The type of the packet (perfect forwarding).
   * @param packet The packet to broadcast.
   */
  template <typename T>
  void BroadcastTcp(T&& packet) {
    for (auto it = connections_.begin(); it != connections_.end();) {
      if ((*it)->IsConnected()) {
        (*it)->Send(std::forward<T>(packet));
        ++it;
      } else {
        OnClientDisconnect(*it);
        it = connections_.erase(it);
      }
    }
  }

  /**
   * @brief Broadcasts a packet to all known UDP endpoints.
   * @tparam T The type of the packet (perfect forwarding).
   * @param packet The packet to broadcast.
   */
  template <typename T>
  void BroadcastUdp(T&& packet) {
    for (const auto& [endpoint, conn] : udp_to_tcp_map_) {
      udp_connection_->SendTo(std::forward<T>(packet), endpoint);
    }
  }

  /**
   * @brief Broadcasts a packet to all TCP connections except one.
   * @tparam T The type of the packet.
   * @param excluded_connection The connection to exclude.
   * @param packet The packet to broadcast.
   */
  template <typename T>
  void BroadcastToOthersTcp(const std::shared_ptr<TcpServerConnection<PacketType>>& excluded_connection,
                            T&& packet) {
    for (auto it = connections_.begin(); it != connections_.end();) {
      if (*it == excluded_connection) {
        ++it;
        continue;
      }
      if ((*it)->IsConnected()) {
        (*it)->Send(std::forward<T>(packet));
        ++it;
      } else {
        OnClientDisconnect(*it);
        it = connections_.erase(it);
      }
    }
  }

  /**
   * @brief Broadcasts a packet to all UDP endpoints except a specific one.
   * @tparam T The type of the packet.
   * @param excluded_endpoint The UDP endpoint to exclude from broadcasting.
   * @param packet The packet to broadcast.
   */
  template <typename T>
  void BroadcastToOthersUdp(const asio::ip::udp::endpoint& excluded_endpoint,
                            T&& packet) {
    for (const auto& [endpoint, conn] : udp_to_tcp_map_) {
      if (endpoint != excluded_endpoint) {
        udp_connection_->SendTo(std::forward<T>(packet), endpoint);
      }
    }
  }

  /**
   * @brief Sends a packet to a specific TCP connection by its ID.
   * @tparam T The type of the packet.
   * @param connection_id The ID of the connection to send to.
   * @param packet The packet to send.
   * @return true if sent successfully, false otherwise.
   */
  template <typename T>
  bool SendToTcp(uint32_t connection_id, T&& packet) {
    bool sent = false;
    for (auto it = connections_.begin(); it != connections_.end();) {
      if ((*it)->GetId() == connection_id) {
        if ((*it)->IsConnected()) {
          (*it)->Send(std::forward<T>(packet));
          sent = true;
          ++it;
        } else {
          OnClientDisconnect(*it);
          it = connections_.erase(it);
        }
      } else {
        ++it;
      }
    }

    if (!sent) {
      std::cerr << "[Server][ERROR] Failed to send to TCP connection ID "
                << connection_id << std::endl;
    }
    return sent;
  }

  /**
   * @brief Sends a packet to a specific UDP endpoint.
   * @tparam T The type of the packet.
   * @param endpoint The UDP endpoint to send to.
   * @param packet The packet to send.
   */
  template <typename T>
  void SendToUdp(const asio::ip::udp::endpoint& endpoint, T&& packet) {
    if (udp_connection_) {
      udp_connection_->SendTo(std::forward<T>(packet), endpoint);
    }
  }

  /**
   * @brief Registers a UDP endpoint for a given TCP connection.
   * @param connection The TCP connection to link with the UDP endpoint.
   * @param udp_port The client's UDP port.
   */
  virtual void RegisterUdpEndpoint(const std::shared_ptr<TcpServerConnection<PacketType>>& connection,
                                   uint16_t udp_port)
  {
    auto client_ip = connection->GetSocket().remote_endpoint().address();
    asio::ip::udp::endpoint udp_endpoint(client_ip, udp_port);

    udp_to_tcp_map_[udp_endpoint] = connection;
    tcp_to_udp_map_[connection]   = udp_endpoint;

    std::cout << "[Server][INFO] Registered UDP endpoint for connection ID "
              << connection->GetId() << " at " << udp_endpoint.address().to_string()
              << ":" << udp_endpoint.port() << std::endl;
  }

  /**
   * @brief Pops the next received packet from the internal queue (if any).
   * @return std::optional containing the OwnedPacket if available, otherwise empty.
   */
  std::optional<OwnedPacket<PacketType>> PopMessage() {
    return received_queue_.Pop();
  }

  /**
   * @brief Returns the number of active TCP connections.
   */
  [[nodiscard]] size_t GetConnectionCount() const {
    return connections_.size();
  }

 protected:
  /**
   * @brief Called when a new TCP client connects (before acceptance).
   * @param connection The incoming TCP connection.
   * @return true if accepted, false if rejected (the socket will be closed).
   */
  virtual bool OnClientConnect(const std::shared_ptr<TcpServerConnection<PacketType>>& connection) {
    static_cast<void>(connection);
    return true;
  }

  /**
   * @brief Called right after a new TCP connection is successfully started.
   * @param connection The accepted connection.
   */
  virtual void OnClientAccepted(const std::shared_ptr<TcpServerConnection<PacketType>>& connection) = 0;

  /**
   * @brief Called when a TCP client disconnects or is found disconnected.
   * @param connection The disconnected connection.
   */
  virtual void OnClientDisconnect(const std::shared_ptr<TcpServerConnection<PacketType>>& connection) {
    // Remove from any UDP mapping if present
    auto it = tcp_to_udp_map_.find(connection);
    if (it != tcp_to_udp_map_.end()) {
      auto udp_endpoint = it->second;
      udp_to_tcp_map_.erase(udp_endpoint);
      tcp_to_udp_map_.erase(it);
    }
  }

  // Mappings UDP <-> TCP
  std::unordered_map<asio::ip::udp::endpoint,
                     std::shared_ptr<TcpServerConnection<PacketType>>>
      udp_to_tcp_map_;  ///< Map linking UDP endpoints to TCP connections.
  std::unordered_map<std::shared_ptr<TcpServerConnection<PacketType>>,
                     asio::ip::udp::endpoint>
      tcp_to_udp_map_;  ///< Map linking TCP connections to UDP endpoints.

 private:
  /**
   * @brief Asynchronously accepts new TCP connections, taking a shared_ptr to the server.
   * @param self A shared pointer to this NetworkServer instance.
   */
  void Accept(std::shared_ptr<NetworkServer> self) {
    acceptor_.async_accept(
      [self](const std::error_code& ec, asio::ip::tcp::socket socket) {
        if (!ec) {

          try {
            const auto client_ip =
                socket.remote_endpoint().address().to_string();
            const auto client_port = socket.remote_endpoint().port();
            std::cout << "[DEBUG] Client connected: " << client_ip << ":" << client_port << std::endl;
          } catch (const std::exception& e) {
            std::cerr << "[DEBUG][ERROR] Unable to retrieve client endpoint: " << e.what() << std::endl;
          }

          auto connection = std::make_shared<TcpServerConnection<PacketType>>(
              std::move(socket),
              self->received_queue_,
              ++(self->connection_id_counter_));

          if (self->OnClientConnect(connection)) {
            self->connections_.push_back(connection);
            connection->Start();
            self->OnClientAccepted(connection);
          } else {
            socket.close();
          }
        } else {
          std::cerr << "[Server][ERROR] Accept error: " << ec.message() << std::endl;
        }

        self->Accept(self);
      }
    );
  }

  /**
   * @brief Schedules the periodic check timer to call CheckConnections() every 'check_interval_ms_' ms.
   * @param self Shared pointer to this NetworkServer.
   */
  void StartCheckTimer(std::shared_ptr<NetworkServer> self) {
    self->check_timer_.expires_after(std::chrono::milliseconds(self->check_interval_ms_));

    self->check_timer_.async_wait([self](const std::error_code &ec) {
      if (!ec) {
        self->CheckConnections();

        self->StartCheckTimer(self);
      } else {
        // If ec == asio::error::operation_aborted, Stop() was called
        // We do nothing here.
      }
    });
  }

  /**
   * @brief Disconnects all TCP clients and clears the connection list.
   */
  void DisconnectAllClients() {
    for (auto& conn : connections_) {
      if (conn->IsConnected()) {
        conn->Disconnect();
      }
    }
    connections_.clear();
  }

  /**
   * @brief Checks all active connections for disconnection and removes them.
   */
  void CheckConnections() {
    for (auto it = connections_.begin(); it != connections_.end();) {
      if (!(*it)->IsConnected()) {
        OnClientDisconnect(*it);
        it = connections_.erase(it);
      } else {
        ++it;
      }
    }
  }

  // Core ASIO objects
  asio::io_context
      io_context_;  ///< ASIO I/O context for managing asynchronous operations.
  asio::ip::tcp::acceptor
      acceptor_;  ///< ASIO TCP acceptor for handling incoming TCP connections.
  std::thread context_thread_;  ///< Thread running the ASIO context.

  // UDP
  std::shared_ptr<UdpServerConnection<PacketType>>
      udp_connection_;  ///< UDP connection for managing UDP communication.

  // Received messages queue
  ConcurrentQueue<OwnedPacket<PacketType>>
      received_queue_;  ///< Queue to store received packets for processing.

  // List of active TCP connections
  std::vector<std::shared_ptr<TcpServerConnection<PacketType>>>
      connections_;  ///< List of active TCP client connections.

  // Connection ID counter
  uint32_t connection_id_counter_{
      0};  ///< Counter for assigning unique IDs to TCP connections.

  // Timer for periodic checks
  asio::steady_timer check_timer_;  ///< Timer for periodic connection checks.
  uint64_t
      check_interval_ms_;  ///< Interval for connection checks in milliseconds.
};

}  // namespace network

#endif  // NETWORK_SERVER_H_
