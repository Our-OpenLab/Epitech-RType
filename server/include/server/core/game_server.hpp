#ifndef SERVER_CORE_GAME_SERVER_H_
#define SERVER_CORE_GAME_SERVER_H_

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <shared/network_messages.hpp>
#include <thread>

#include "server/core/custom_network_server.hpp"
#include "server/core/event_queue.hpp"
#include "server/core/message_dispatcher.hpp"
#include "server/engine/game_engine.hpp"
#include "server/engine/game_state.hpp"

namespace game {

template <typename PacketType>
class GameServer {
public:
  explicit GameServer(uint16_t tcp_port, uint16_t udp_port);
  ~GameServer();

  bool Start();
  void Stop();

private:
  static constexpr std::chrono::milliseconds kTickDuration{8};  // 16 ms (~62.5 ticks/sec)
  static constexpr int kMaxPacketsPerTick{50};
  static constexpr std::chrono::milliseconds kMaxPacketProcessingTime{10};
  static constexpr std::size_t kSafeUdpPayloadSize = 512;
  //static constexpr std::size_t kUpdatePositionSize = sizeof(network::UpdatePosition);
  static constexpr std::size_t kUpdatePositionSize = sizeof(network::UpdatePlayer);
  static constexpr std::size_t kMaxUpdatesPerPacket = kSafeUdpPayloadSize / kUpdatePositionSize;
  static constexpr uint32_t kUpdateFrequencyTicks = 1;

  void Run();
  void ProcessPackets(int max_packets, std::chrono::milliseconds max_time);
  void SendUpdatesToClients(uint32_t timestamp);
  void SendPlayerUpdates(uint32_t timestamp);
  void SendProjectileUpdates(uint32_t timestamp);
  void SendEnemyUpdates(uint32_t timestamp);
  template <typename T>
  void SendUpdatePacket(const std::array<T, kMaxUpdatesPerPacket>& updates, size_t count, PacketType type);

  network::CustomNetworkServer<PacketType> network_server_;
  GameEngine game_engine_;
  GameState game_state_;
  EventQueue event_queue_;
  std::thread game_thread_;
  bool running_ = false;
};

}  // namespace game

#include "server/core/game_server.tpp"

#endif  // SERVER_CORE_GAME_SERVER_H_
