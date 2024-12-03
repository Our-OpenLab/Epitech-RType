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
  uint32_t timestamp;
};

struct UpdatePosition {
  uint8_t player_id;
  float x, y;
};
}

#pragma pack(pop)

#endif  // NETWORK_MESSAGES_HPP_
