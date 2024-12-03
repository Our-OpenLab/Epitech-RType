#ifndef MESSAGE_DISPATCHER_HPP_
#define MESSAGE_DISPATCHER_HPP_

#include <array>
#include <functional>
#include <network/protocol.hpp>
#include <shared/my_packet_types.hpp>
#include <shared/network_messages.hpp>

#include "client.hpp"

class MessageDispatcher {
public:
  using Handler = std::function<void(network::Packet<network::MyPacketType>&, Client&)>;

  static void dispatch(network::Packet<network::MyPacketType>&& packet, Client& client) {
    const auto index = static_cast<size_t>(packet.header.type);

    if (index >= handlers_.size() || !handlers_[index]) {
      default_handler(packet, client);
      return;
    }
    handlers_[index](packet, client);
  }

  static const std::array<Handler, static_cast<size_t>(network::MyPacketType::MaxTypes)> handlers_;

private:

  static void handle_pong(network::Packet<network::MyPacketType>& packet, Client& client) {
    try {
      if (packet.body.size() == sizeof(std::uint32_t)) {
        const auto timestamp = packet.extract<std::uint32_t>();

        const auto& time_manager = client.get_time_manager();
        const auto current_time = time_manager.now();
        const auto received_time = std::chrono::milliseconds(timestamp);

        const auto ping = std::chrono::duration_cast<std::chrono::milliseconds>(
                        current_time.time_since_epoch())
                        .count() -
                    received_time.count();

        client.get_ping_manager().set_ping(static_cast<int>(ping));

        std::cout << "[MessageDispatcher][INFO] Ping updated: "
                  << client.get_ping_manager().get_ping() << " ms\n";
      } else {
        std::cerr << "[MessageDispatcher][ERROR] Pong packet has insufficient data.\n";
      }
    } catch (const std::exception& e) {
      std::cerr << "[MessageDispatcher][ERROR] Exception while handling Pong: " << e.what() << '\n';
    }
  }


  static void handle_player_assign(const network::Packet<network::MyPacketType>& packet, Client& client) {
    auto assign_message = network::PacketFactory<network::MyPacketType>::extract_data<network::PlayerAssign>(packet);
    client.client_id_ = assign_message.player_id;

    std::cout << "[Client][INFO] Assigned Player ID: "
              << static_cast<int>(client.client_id_) << std::endl;

    client.get_game_state().add_player(assign_message.player_id, 100.0f, 100.0f, 5.0f);
  }

  static void default_handler(network::Packet<network::MyPacketType>& packet, Client& client) {
    std::cout << "[Client][WARNING] Unhandled packet type: "
              << static_cast<int>(packet.header.type) << std::endl;
  }

  static void handle_update_position(network::Packet<network::MyPacketType>& packet, Client& client) {
    auto update_position = network::PacketFactory<network::MyPacketType>::extract_data<network::UpdatePosition>(packet);

    auto& game_state = client.get_game_state();
    auto& player = game_state.get_player(update_position.player_id);

    player.x = update_position.x;
    player.y = update_position.y;

    std::cout << "[Client][INFO] Updated position for Player "
              << static_cast<int>(update_position.player_id) << " to (" << player.x << ", " << player.y << ")\n";
  }

};

const std::array<MessageDispatcher::Handler, static_cast<size_t>(network::MyPacketType::MaxTypes)> MessageDispatcher::handlers_ = {
  default_handler, // ChatMessage
  default_handler, // PlayerMove
  default_handler, // Disconnect
  default_handler, // Ping
  handle_pong, // Pong
  default_handler, // PlayerInput
  handle_update_position, // UpdatePosition
  &MessageDispatcher::handle_player_assign, // PlayerAssign
};

#endif  // MESSAGE_DISPATCHER_HPP_
