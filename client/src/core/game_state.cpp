#include <algorithm>
#include <client/core/game_state.hpp>
#include <iostream>
#include <ostream>
#include <stdexcept>

void GameState::add_player(uint8_t id, float x, float y, float speed) {
  //std::lock_guard<std::mutex> lock(mutex_);
  players_[id] = Player{id, x, y, speed, true};
}

void GameState::remove_player(uint8_t id) {
  //std::lock_guard<std::mutex> lock(mutex_);
  std::cout << id << " removed" << std::endl;
  players_.erase(id);
}

Player& GameState::get_player(uint8_t id) {
  //std::lock_guard<std::mutex> lock(mutex_);
  // can be made faster
  if (!players_.contains(id)) {
    throw std::runtime_error("Player not found");
  }
  return players_.at(id);
}

const std::unordered_map<uint8_t, Player>& GameState::get_players() const {
  return players_;
}
