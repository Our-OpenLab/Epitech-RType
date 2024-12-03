#ifndef PLAYER_ACTIONS_HPP_
#define PLAYER_ACTIONS_HPP_

#include <cstdint>

enum class PlayerAction : uint16_t {
  MoveUp = 1 << 0,
  MoveDown = 1 << 1,
  MoveLeft = 1 << 2,
  MoveRight = 1 << 3,
  Shoot = 1 << 4,
};

namespace player_actions {

inline uint16_t operator|(const uint16_t lhs, PlayerAction rhs) {
  return lhs | static_cast<uint16_t>(rhs);
}

inline uint16_t operator&(const uint16_t lhs, PlayerAction rhs) {
  return lhs & static_cast<uint16_t>(rhs);
}

inline uint16_t& operator|=(uint16_t& lhs, PlayerAction rhs) {
  lhs |= static_cast<uint16_t>(rhs);
  return lhs;
}

inline uint16_t& operator&=(uint16_t& lhs, PlayerAction rhs) {
  lhs &= static_cast<uint16_t>(rhs);
  return lhs;
}

inline uint16_t operator~(PlayerAction action) {
  return ~static_cast<uint16_t>(action);
}

}

#endif // PLAYER_ACTIONS_HPP_
