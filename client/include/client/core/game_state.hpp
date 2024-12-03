#ifndef GAME_STATE_HPP_
#define GAME_STATE_HPP_

#include <unordered_map>
#include <cstdint>
#include <mutex>

#include "player.hpp"

class GameState {
public:
  void add_player(uint8_t id, float x, float y, float speed);
  void remove_player(uint8_t id);
  [[nodiscard]] Player& get_player(uint8_t id);
  [[nodiscard]] const std::unordered_map<uint8_t, Player>& get_players() const;

private:
  std::unordered_map<uint8_t, Player> players_;
  //mutable std::mutex mutex_;
};

#endif // GAME_STATE_HPP_
