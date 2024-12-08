#ifndef CLIENT_HPP_
#define CLIENT_HPP_

#include <network/network_client.hpp>
#include <shared/my_packet_types.hpp>

#include "client/engine/game_engine.hpp"
#include "client/engine/game_state.hpp"
#include "client/core/input_manager.hpp"
#include "client/core/renderer.hpp"

class Client {
  friend class MessageDispatcher;

public:
  Client(const std::string& host, const std::string& port);
  ~Client();

  void Run();
  void Shutdown();

  client::GameState& GetGameState() { return game_state_; }

private:
  static constexpr std::chrono::milliseconds kTickDuration{16};  // 16 ms (~62.5 ticks/sec)
  static constexpr uint64_t kPingFrequencyTicks{60};
  static constexpr int kMaxPacketsPerTick{50};
  static constexpr std::chrono::milliseconds kMaxPacketProcessingTime{10};
  static constexpr std::chrono::milliseconds kRenderDelay{100};

  void ProcessPackets(int max_packets, std::chrono::milliseconds max_time);
  void SendPing(uint64_t tick_counter);

  bool is_running_{true};
  uint8_t client_id_{0};

  Renderer renderer_;
  network::NetworkClient<network::MyPacketType> network_client_;
  InputManager input_manager_;
  ScreenManager screen_manager_{};
  client::GameEngine game_engine_{};
  client::GameState game_state_;
};

#endif  // CLIENT_HPP_
