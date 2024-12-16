#ifndef NETWORK_MESSAGES_HPP_
#define NETWORK_MESSAGES_HPP_

#include <cstdint>

#pragma pack(push, 1)

namespace network {
struct PlayerAssign {
  uint8_t player_id;
  float spawn_x;
  float spawn_y;
  uint16_t score;
};

struct PlayerInput {
  uint8_t player_id;
  uint16_t actions;
  float dir_x;
  float dir_y;
  uint32_t timestamp;
};

struct PlayerJoin {
  uint8_t player_id;
  float x, y;
  uint16_t score;
};

struct PlayerLeave {
  uint8_t player_id;
};

struct RemoveProjectile {
  uint8_t projectile_id;
};

struct RemoveEnemy {
  uint8_t enemy_id;
};

struct UpdatePlayer {
  uint8_t player_id;
  float x;
  float y;
  uint16_t score;
};

struct UpdateEnemy {
  uint8_t enemy_id;
  float x;
  float y;
};

struct UpdateProjectile {
  uint8_t projectile_id;
  uint8_t owner_id;
  float x;
  float y;
};

struct UdpPort {
  uint16_t port;
};

}

#pragma pack(pop)

#endif  // NETWORK_MESSAGES_HPP_
