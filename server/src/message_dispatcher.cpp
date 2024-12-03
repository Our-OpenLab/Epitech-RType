#include <game/message_dispatcher.hpp>
#include <shared/my_packet_types.hpp>
#include <shared/network_messages.hpp>

namespace game {

void MessageDispatcher::default_handler(network::Packet<network::MyPacketType>& packet, const std::shared_ptr<network::ServerConnection<network::MyPacketType>>&, GameState& game_state) {
  std::cerr << "[MessageDispatcher] Unhandled packet type: "
            << static_cast<int>(packet.header.type) << "\n";
}

void handle_ping(network::Packet<network::MyPacketType>& packet, const std::shared_ptr<network::ServerConnection<network::MyPacketType>>& connection, GameState& game_state) {
  try {
    if (packet.body.size() == sizeof(std::uint32_t)) {
      const auto timestamp = packet.extract<std::uint32_t>();
      std::cout << "[MessageDispatcher] Ping received with timestamp: "
                << timestamp << "\n";

      network::Packet<network::MyPacketType> pong_packet;
      pong_packet.header.type = network::MyPacketType::Pong;
      pong_packet.push<std::uint32_t>(timestamp);

      connection->send(pong_packet);

      std::cout << "[MessageDispatcher] Pong sent with timestamp: "
                << timestamp << "\n";
    } else {
      std::cerr << "[MessageDispatcher] Ping packet has insufficient data.\n";
    }
  } catch (const std::exception& e) {
    std::cerr << "[MessageDispatcher][ERROR] " << e.what() << "\n";
  }
}

void handle_player_input(network::Packet<network::MyPacketType>& packet, const std::shared_ptr<network::ServerConnection<network::MyPacketType>>& connection, GameState& game_state) {
  try {
    if (packet.body.size() != sizeof(network::PlayerInput)) {
      std::cerr << "[MessageDispatcher] PlayerInput packet has insufficient data.\n";
      return;
    }

    auto input = network::PacketFactory<network::MyPacketType>::extract_data<network::PlayerInput>(packet);
    auto& player = game_state.get_player(input.player_id);

    player.actions = input.actions;

    std::cout << "[MessageDispatcher] Player " << static_cast<int>(input.player_id)
              << " actions updated: " << input.actions << "\n";
  } catch (const std::exception& e) {
    std::cerr << "[MessageDispatcher][ERROR] Failed to process PlayerInput: " << e.what() << "\n";
  }
}

const std::array<MessageDispatcher::Handler, static_cast<size_t>(network::MyPacketType::MaxTypes)> MessageDispatcher::handlers_ = {
  default_handler,
  default_handler,
  default_handler,
  handle_ping,
  default_handler,
  handle_player_input,
  default_handler, // UpdatePosition
  default_handler, // PlayerAssign
};
}
