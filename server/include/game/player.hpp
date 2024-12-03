#ifndef PLAYER_HPP_
#define PLAYER_HPP_

#include <string>

struct Player {
  uint8_t id;
  float x, y;
  float speed;
  uint16_t actions;
  int health;
  int score;
};

#endif // PLAYER_HPP_
