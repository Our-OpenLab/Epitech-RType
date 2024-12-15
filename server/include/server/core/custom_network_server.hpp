#ifndef CUSTOM_NETWORK_SERVER_HPP_
#define CUSTOM_NETWORK_SERVER_HPP_

#include <memory>
#include <network/network_server.hpp>
#include "event_queue.hpp"
#include "server/engine/game_state.hpp"

namespace network {
class MessageDispatcher;
}

namespace network {

template <typename PacketType>
class CustomNetworkServer final : public NetworkServer<PacketType> {
public:
  explicit CustomNetworkServer(uint16_t tcp_port, uint16_t udp_port, GameState& game_state, EventQueue& event_queue);

  void process_message(OwnedPacket<PacketType>&& owned_packet);

  [[nodiscard]] GameState& GetGameState() const;

  [[nodiscard]] MessageDispatcher& GetMessageDispatcher() const;

protected:
  void OnClientAccepted(const std::shared_ptr<TcpServerConnection<PacketType>>& connection) override;
  void OnClientDisconnect(const std::shared_ptr<TcpServerConnection<PacketType>>& connection) override;

private:
  GameState& game_state_;
  EventQueue& event_queue_;
  std::unique_ptr<MessageDispatcher> message_dispatcher_;
};

}  // namespace network

#endif  // CUSTOM_NETWORK_SERVER_HPP_
