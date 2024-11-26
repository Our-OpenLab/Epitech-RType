#include <game/message_dispatcher.hpp>

namespace game {

void MessageDispatcher::default_handler(network::Packet& packet, const std::shared_ptr<network::ServerConnection>&) {
  std::cerr << "[MessageDispatcher] Unhandled packet type: "
            << static_cast<int>(packet.header.type) << "\n";
}

void handle_ping(network::Packet& packet, const std::shared_ptr<network::ServerConnection>& connection) {
  try {
    if (packet.body.size() == sizeof(std::uint32_t)) {
      const auto timestamp = packet.extract<std::uint32_t>();
      std::cout << "[MessageDispatcher] Ping received with timestamp: "
                << timestamp << "\n";

      network::Packet pong_packet;
      pong_packet.header.type = network::PacketType::Pong;
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

const std::array<MessageDispatcher::Handler, static_cast<size_t>(network::PacketType::MaxTypes)> MessageDispatcher::handlers_ = {
  default_handler,
  default_handler,
  handle_ping,
  default_handler
};
}
