#ifndef NETWORK_SERVER_H_
#define NETWORK_SERVER_H_

#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>

#include <algorithm>
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
 * @brief Manages a network server with both TCP and UDP support.
 *
 * Handles client connections, message routing, and broadcasting for both
 * TCP and UDP protocols.
 *
 * @tparam PacketType The type of packets used in the server.
 */
template <typename PacketType>
class NetworkServer {
 public:
  /**
   * @brief Constructs a network server.
   *
   * @param tcp_port Port for TCP connections.
   * @param udp_port Port for UDP communication.
   */
  explicit NetworkServer(const uint16_t tcp_port, const uint16_t udp_port)
      : acceptor_(io_context_,
                  asio::ip::tcp::endpoint(asio::ip::tcp::v4(), tcp_port)),
        udp_connection_(std::make_shared<UdpServerConnection<PacketType>>(
            io_context_, udp_port, received_queue_)) {}

  /**
   * @brief Destructor to clean up resources.
   */
  virtual ~NetworkServer() { Stop(); }

  /**
   * @brief Starts the network server.
   *
   * Begins accepting TCP connections and initializes UDP communication.
   *
   * @return True if the server started successfully, false otherwise.
   */
  bool Start() {
    try {
      Accept();
      if (udp_connection_) {
        udp_connection_->Start();
      }
      context_thread_ = std::thread([this]() { io_context_.run(); });
      std::cout << "[Server][INFO] Started successfully on TCP port "
                << acceptor_.local_endpoint().port() << std::endl;
      return true;
    } catch (const std::exception& e) {
      std::cerr << "[Server][ERROR] Failed to start: " << e.what() << std::endl;
      return false;
    }
  }

  /**
   * @brief Stops the network server and disconnects all clients.
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
   * @brief Broadcasts a packet to all TCP connections.
   *
   * @tparam T The type of the packet.
   * @param packet The packet to broadcast.
   */
  template <typename T>
  void BroadcastTcp(T&& packet) {
      std::erase_if(connections_, [this, &packet](const auto& connection) {
        if (connection->IsConnected()) {
          connection->Send(std::forward<T>(packet));
          return false;
        }
        OnClientDisconnect(connection);
        return true;
      });
    }

  /**
   * @brief Broadcasts a packet to all UDP endpoints.
   *
   * @tparam T The type of the packet.
   * @param packet The packet to broadcast.
   */
  template <typename T>
  void BroadcastUdp(T&& packet) {
    for (const auto& [udp_endpoint, connection] : udp_to_tcp_map_) {
      udp_connection_->SendTo(std::forward<T>(packet), udp_endpoint);
    }
  }

  /**
   * @brief Broadcasts a packet to all TCP connections except one.
   *
   * @tparam T The type of the packet.
   * @param excluded_connection The connection to exclude.
   * @param packet The packet to broadcast.
   */
  template <typename T>
  void BroadcastToOthersTcp(
    const std::shared_ptr<TcpServerConnection<PacketType>>& excluded_connection,
    T&& packet) {
    std::erase_if(connections_, [this, &packet, &excluded_connection](const auto& connection) {
      if (connection == excluded_connection) {
        return false;
      }
      if (connection->IsConnected()) {
        connection->Send(std::forward<T>(packet));
        return false;
      }
      OnClientDisconnect(connection);
      return true;
    });
  }


  /**
  * @brief Broadcasts a packet to all UDP endpoints except one.
  *
  * @tparam T The type of the packet.
  * @param excluded_endpoint The endpoint to exclude.
  * @param packet The packet to broadcast.
  */
  template <typename T>
  void BroadcastToOthersUdp(const asio::ip::udp::endpoint& excluded_endpoint,
                            T&& packet) {
    for (const auto& [udp_endpoint, connection] : udp_to_tcp_map_) {
      if (udp_endpoint != excluded_endpoint) {
        udp_connection_->SendTo(std::forward<T>(packet), udp_endpoint);
      }
    }
  }

  /**
   * @brief Sends a packet to a specific TCP connection by ID.
   *
   * @tparam T The type of the packet.
   * @param connection_id The ID of the target connection.
   * @param packet The packet to send.
   * @return True if the packet was sent, false otherwise.
   */
  template <typename T>
  bool SendToTcp(uint32_t connection_id, T&& packet) {
    bool sent = false;

    std::erase_if(connections_, [this, &packet, connection_id, &sent](const auto& connection) {
        if (connection->GetId() == connection_id) {
            if (connection->IsConnected()) {
                connection->Send(std::forward<T>(packet));
                sent = true;
                return false;
            } else {
                OnClientDisconnect(connection);
                return true;
            }
        }
        return false;
    });

    if (!sent) {
        std::cerr << "[Server][ERROR] Failed to send packet to TCP connection ID "
                  << connection_id << std::endl;
    }
    return sent;
  }

  /**
   * @brief Sends a packet to a specific UDP endpoint.
   *
   * @tparam T The type of the packet.
   * @param endpoint The target UDP endpoint.
   * @param packet The packet to send.
   */
  template <typename T>
  void SendToUdp(const asio::ip::udp::endpoint& endpoint, T&& packet) {
    if (udp_connection_) {
      udp_connection_->SendTo(std::forward<T>(packet), endpoint);
    }
  }

  /**
   * @brief Registers a UDP endpoint for a TCP connection.
   *
   * @param connection The TCP connection to associate with the UDP endpoint.
   * @param udp_port The UDP port of the client.
   */
  virtual void RegisterUdpEndpoint(
      const std::shared_ptr<TcpServerConnection<PacketType>>& connection,
      uint16_t udp_port) {
    auto client_ip = connection->GetSocket().remote_endpoint().address();
    asio::ip::udp::endpoint udp_endpoint(client_ip, udp_port);

    udp_to_tcp_map_[udp_endpoint] = connection;
    tcp_to_udp_map_[connection] = udp_endpoint;

    std::cout << "[Server][INFO] Registered UDP endpoint for connection ID "
              << connection->GetId() << " at " << udp_endpoint.address().to_string()
              << ":" << udp_endpoint.port() << std::endl;
  }

  /**
   * @brief Checks all active connections and removes disconnected ones.
   *
   * Iterates through the list of TCP connections and verifies their status.
   * If a connection is no longer active, it triggers the `OnClientDisconnect` callback
   * and removes the connection from the list.
   *
   * This function ensures that only active connections are maintained,
   * freeing resources associated with disconnected clients.
   */
  void CheckConnections() {
    std::erase_if(connections_, [this](const auto& connection) {
        if (!connection->IsConnected()) {
         //   std::cout << "[Server][INFO] Removing disconnected client ID "
         //             << connection->GetId() << std::endl;
            OnClientDisconnect(connection);
            return true;
        }
        return false;
    });
  }


  /**
   * @brief Pops a received packet from the queue.
   *
   * @return An optional containing the packet if available.
   */
  std::optional<OwnedPacket<PacketType>> PopMessage() {
    return received_queue_.Pop();
  }

  /**
   * @brief Gets the number of active connections.
   *
   * @return The connection count.
   */
  [[nodiscard]] size_t GetConnectionCount() const {
    return connections_.size();
  }

 protected:
  virtual bool OnClientConnect(
      const std::shared_ptr<TcpServerConnection<PacketType>>& connection) {
    return true;
  }

  virtual void OnClientAccepted(
      const std::shared_ptr<TcpServerConnection<PacketType>>& connection) {}

  virtual void OnClientDisconnect(
    const std::shared_ptr<TcpServerConnection<PacketType>>& connection) {
    // std::cout << "[Server][INFO] Client with ID " << connection->GetId()
    //         << " disconnected." << std::endl;

    auto tcp_to_udp_it = tcp_to_udp_map_.find(connection);
    if (tcp_to_udp_it != tcp_to_udp_map_.end()) {
      const auto& udp_endpoint = tcp_to_udp_it->second;

      udp_to_tcp_map_.erase(udp_endpoint);

      tcp_to_udp_map_.erase(tcp_to_udp_it);

      // std::cout << "[Server][INFO] Removed UDP endpoint mapping for client ID "
      //           << connection->GetId() << " at " << udp_endpoint.address().to_string()
      //           << ":" << udp_endpoint.port() << std::endl;
    }
  }

  std::unordered_map<asio::ip::udp::endpoint, std::shared_ptr<TcpServerConnection<PacketType>>> udp_to_tcp_map_;  ///< Map linking UDP endpoints to TCP connections.
  std::unordered_map<std::shared_ptr<TcpServerConnection<PacketType>>, asio::ip::udp::endpoint> tcp_to_udp_map_;  ///< Map linking TCP connections to UDP endpoints.

 private:
  void Accept() {
    acceptor_.async_accept(
        [this](const std::error_code& ec, asio::ip::tcp::socket socket) {
          if (!ec) {
            auto connection = std::make_shared<TcpServerConnection<PacketType>>(
                std::move(socket), received_queue_, ++connection_id_counter_);

            if (OnClientConnect(connection)) {
              connections_.push_back(connection);
              connection->Start();
              OnClientAccepted(connection);
            } else {
              socket.close();
            }
          } else {
            std::cerr << "[Server][ERROR] Accept error: " << ec.message()
                      << std::endl;
          }
          Accept();
        });
  }

  void DisconnectAllClients() {
    for (const auto& connection : connections_) {
      if (connection->IsConnected()) {
        connection->Disconnect();
      }
    }
    connections_.clear();
  }

  asio::io_context io_context_;  ///< ASIO I/O context for managing asynchronous operations.
  asio::ip::tcp::acceptor acceptor_;  ///< ASIO TCP acceptor for handling incoming TCP connections.
  std::shared_ptr<UdpServerConnection<PacketType>> udp_connection_;  ///< UDP connection for managing UDP communication.
  ConcurrentQueue<OwnedPacket<PacketType>> received_queue_;  ///< Queue to store received packets for processing.
  std::vector<std::shared_ptr<TcpServerConnection<PacketType>>> connections_;  ///< List of active TCP client connections.
  std::thread context_thread_;  ///< Thread running the ASIO context.
  uint32_t connection_id_counter_ = 0;  ///< Counter for assigning unique IDs to TCP connections.

};

}  // namespace network

#endif  // NETWORK_SERVER_H_
