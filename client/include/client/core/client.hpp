#ifndef CLIENT_HPP_
#define CLIENT_HPP_

#include <network/network_client.hpp>
#include <shared/my_packet_types.hpp>

#include "renderer.hpp"
#include "time_manager.hpp"
#include "ping_manager.hpp"
#include "game_state.hpp"

class Client {
  friend class MessageDispatcher;
public:
  Client(const std::string& host, const std::string& port);

  ~Client();

  void run();

  void handle_input(const SDL_Event& event);

  void process_packets(int max_packets, std::chrono::milliseconds max_time);

  void shutdown();

  network::NetworkClient<network::MyPacketType>& get_network_client() {
    return network_client_;
  }

  TimeManager& get_time_manager() {
    return time_manager_;
  }

  PingManager& get_ping_manager() {
    return ping_manager_;
  }

  GameState& get_game_state() {
    return game_state_;
  }

private:
  bool is_running_ = true;
  Renderer renderer_;
  network::NetworkClient<network::MyPacketType> network_client_;

  TimeManager time_manager_;
  PingManager ping_manager_;
  GameState game_state_;
  uint8_t client_id_ = 0;
};

#endif //CLIENT_HPP_
