#ifndef MY_PACKET_TYPES_HPP_
#define MY_PACKET_TYPES_HPP_

#include <cstdint>

namespace network {
enum class MyPacketType : uint32_t {
  ChatMessage,
  PlayerMove,
  Disconnect,
  Ping,
  Pong,
  PlayerInput,
  UpdatePosition,
  PlayerAssign,
  MaxTypes
};
}

#endif  // MY_PACKET_TYPES_HPP_
