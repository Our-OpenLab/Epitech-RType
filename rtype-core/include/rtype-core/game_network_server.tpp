#ifndef GAME_NETWORK_SERVER_TPP_
#define GAME_NETWORK_SERVER_TPP_

#include <iostream>

namespace network {

template <typename PacketType>
GameNetworkServer<PacketType>::GameNetworkServer(uint16_t tcp_port,
                                                 uint16_t udp_port,
                                                 rtype::EventQueue& event_queue)
    : NetworkServer<PacketType>(tcp_port, udp_port),
      event_queue_(event_queue) {}

template <typename PacketType>
void GameNetworkServer<PacketType>::OnClientAccepted(
    const std::shared_ptr<TcpServerConnection<PacketType>>& connection) {
  std::cout << "[GameNetworkServer] Client accepted: " << connection->GetId() << std::endl;

  Packet<PacketType> welcome_packet;
  welcome_packet.header.type = PacketType::kWelcome;

  connection->Send(welcome_packet);
}

template <typename PacketType>
void GameNetworkServer<PacketType>::OnClientDisconnect(
    const std::shared_ptr<TcpServerConnection<PacketType>>& connection) {
  std::cout << "[GameNetworkServer] Client disconnected: " << connection->GetId() << std::endl;

  // Publish an event to the EventQueue
  // auto event = std::make_shared<int>(connection->GetId());
  // event_queue_.Publish(EventType::PlayerDied, event);
}

}  // namespace network

#endif  // GAME_NETWORK_SERVER_TPP_
