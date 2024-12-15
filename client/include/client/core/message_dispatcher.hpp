#ifndef MESSAGE_DISPATCHER_HPP_
#define MESSAGE_DISPATCHER_HPP_

#include <array>
#include <functional>
#include <network/protocol.hpp>
#include <shared/my_packet_types.hpp>
#include <shared/network_messages.hpp>

class Client;

class MessageDispatcher {
public:
  using Handler = std::function<void(network::Packet<network::MyPacketType>&, Client&)>;

  static void Dispatch(network::Packet<network::MyPacketType>&& packet, Client& client);

private:
  static void DefaultHandler(network::Packet<network::MyPacketType>& packet, Client& client);

  static void HandlePlayerAssign(const network::Packet<network::MyPacketType>& packet, Client& client);
  static void HandlePlayerLeave(const network::Packet<network::MyPacketType>& packet, Client& client);
  static void HandlePlayerJoin(const network::Packet<network::MyPacketType>& packet, Client& client);
  static void HandlePong(network::Packet<network::MyPacketType>& packet, Client& client);

  static void HandleUpdatePlayers(const network::Packet<network::MyPacketType>& packet, Client& client);
  static void HandleUpdateEnemies(const network::Packet<network::MyPacketType>& packet, Client& client);
  static void HandleUpdateProjectiles(const network::Packet<network::MyPacketType>& packet, Client& client);
  static void HandleRemoveProjectiles(const network::Packet<network::MyPacketType>& packet, Client& client);

  static const std::array<Handler, static_cast<size_t>(network::MyPacketType::kMaxTypes)> handlers_;
};

#endif  // MESSAGE_DISPATCHER_HPP_
