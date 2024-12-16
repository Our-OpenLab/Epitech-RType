#ifndef CLIENT_HPP_
#define CLIENT_HPP_

#include <network/network_client.hpp>
#include <shared/my_packet_types.hpp>

#include "client/engine/game_engine.hpp"
#include "client/engine/game_state.hpp"
#include "client/core/input_manager.hpp"
#include "client/core/renderer.hpp"

namespace network {
class MessageDispatcher;
}

class Client {


public:
  Client(const std::string& host, const std::string& port, uint16_t udp_port);
  ~Client();

  void Run();
  void Shutdown();

  [[nodiscard]] uint8_t GetClientId() const { return client_id_; }
  void SetClientId(const uint8_t id) { client_id_ = id; }

  client::GameState& GetGameState() { return game_state_; }

  const network::NetworkClient<network::MyPacketType>& GetNetworkClient() const { return network_client_; }

private:
  static constexpr std::chrono::milliseconds kTickDuration{8};  // 16 ms (~62.5 ticks/sec)
  static constexpr uint64_t kPingFrequencyTicks{60};
  static constexpr int kMaxPacketsPerTick{50};
  static constexpr std::chrono::milliseconds kMaxPacketProcessingTime{10};
  static constexpr std::chrono::milliseconds kRenderDelay{100};

  void ProcessPackets(int max_packets, std::chrono::milliseconds max_time);
  void SendPing(uint32_t timestamp) const;

  bool is_running_{true};
  uint8_t client_id_{0};

  Renderer renderer_;
  network::NetworkClient<network::MyPacketType> network_client_;
  InputManager input_manager_;
  ScreenManager screen_manager_{};
  client::GameEngine game_engine_{};
  client::GameState game_state_;
  std::unique_ptr<network::MessageDispatcher> message_dispatcher_;
};

#endif  // CLIENT_HPP_
