#include <client/core/ping_manager.hpp>
#include <client/core/client.hpp>
#include <network/protocol.hpp>
#include <network/protocol.hpp>

void PingManager::update() {
  auto& time_manager = client_.get_time_manager();

  if (time_manager.time_since(last_ping_time_) >= std::chrono::seconds(1)) {
    send_ping();
    last_ping_time_ = time_manager.now();
  }
}

void PingManager::send_ping() {
  auto& time_manager = client_.get_time_manager();

  auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                       time_manager.now().time_since_epoch())
                       .count();

  auto ping_packet = network::PacketFactory<network::MyPacketType>::create_packet(
      network::MyPacketType::Ping, static_cast<uint32_t>(timestamp));

  client_.get_network_client().send(std::move(ping_packet));

  std::cout << "[PingManager][INFO] Ping sent with timestamp: " << timestamp << '\n';
}
