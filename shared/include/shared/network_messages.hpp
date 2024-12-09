#ifndef NETWORK_MESSAGES_HPP_
#define NETWORK_MESSAGES_HPP_

#include <cstdint>

#pragma pack(push, 1)

namespace network {
struct PlayerAssign {
  uint8_t player_id;
};

struct PlayerInput {
  uint8_t player_id;
  uint16_t actions;
  float mouse_x;
  float mouse_y;
  uint32_t timestamp;
};

struct UpdatePosition {
  uint8_t player_id;
  float x;
  float y;
  uint32_t timestamp;
};

struct UpdateProjectile {
  uint8_t projectile_id;
  uint8_t owner_id;
  float x;
  float y;
  uint32_t timestamp;
};

struct PlayerJoin {
  uint8_t player_id;
  float x, y;
};

struct PlayerLeave {
  uint8_t player_id;
};

struct RemoveProjectile {
  uint8_t projectile_id;
};

}

#pragma pack(pop)

#endif  // NETWORK_MESSAGES_HPP_
