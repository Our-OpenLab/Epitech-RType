#ifndef GAME_STATE_HPP_
#define GAME_STATE_HPP_

#include <cstdint>
#include <network/network_server.hpp>
#include <shared/my_packet_types.hpp>
#include <unordered_map>
#include <vector>

#include "player.hpp"

class GameState {
public:
  void add_player(uint8_t id, float x, float y, float speed);

  void remove_player(uint8_t id);

  Player& get_player(uint8_t id);

  void update(float delta_time, network::NetworkServer<network::MyPacketType>& network_server);

  [[nodiscard]] std::vector<Player> get_all_players() const;

private:
  std::unordered_map<uint8_t, Player> players_;
  //mutable std::mutex mutex_;
};

#endif // GAME_STATE_HPP_
