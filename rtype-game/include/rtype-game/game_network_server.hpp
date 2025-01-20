#ifndef GAME_NETWORK_SERVER_HPP_
#define GAME_NETWORK_SERVER_HPP_

#include <memory>
#include <network/network_server.hpp>
#include "event_queue.hpp"

namespace network {

/**
 * @brief A specialized network server class derived from NetworkServer.
 *
 * This server integrates event-based communication on top of
 * the base networking functionality, decoupling the network layer
 * from the game logic.
 *
 * @tparam PacketType The packet type used for networking.
 */
template <typename PacketType>
class GameNetworkServer final : public NetworkServer<PacketType> {
public:
  /**
   * @brief Constructs the GameNetworkServer.
   *
   * @param tcp_port    The TCP port accepting client connections.
   * @param udp_port    The UDP port used for datagram packets.
   * @param event_queue A reference to the shared EventQueue for publishing events.
   */
  explicit GameNetworkServer(uint16_t tcp_port,
                             uint16_t udp_port,
                             rtype::EventQueue& event_queue);

  /**
   * @brief Called after a new client connection is accepted.
   *
   * Publishes an event to notify other systems of the new connection.
   *
   * @param connection The newly accepted client connection.
   */
  void OnClientAccepted(const std::shared_ptr<TcpServerConnection<PacketType>>& connection) override;

  /**
   * @brief Called when a client disconnects or is otherwise lost.
   *
   * Publishes an event to notify other systems of the disconnection.
   *
   * @param connection The disconnected client connection.
   */
  void OnClientDisconnect(const std::shared_ptr<TcpServerConnection<PacketType>>& connection) override;

private:
  rtype::EventQueue& event_queue_;  ///< Reference to the shared EventQueue for publishing events.
};

}  // namespace network

#include "game_network_server.tpp"

#endif  // GAME_NETWORK_SERVER_HPP_
