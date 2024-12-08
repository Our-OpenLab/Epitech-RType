#ifndef MY_PACKET_TYPES_HPP_
#define MY_PACKET_TYPES_HPP_

#include <cstdint>

namespace network {
enum class MyPacketType : uint32_t {
  kPlayerAssign,       // Server -> Client: Assign an ID to the player
  kPlayerInput,        // Client -> Server: Player's input data
  kUpdatePosition,     // Server -> Client: Updated position of a player
  kUpdateProjectile,   // Server -> Client: Updated position of a projectile
  kPlayerJoin,         // Server -> Client: Notification of a new player joining
  kPlayerLeave,        // Server -> Client: Notification of a player leaving
  kDisconnect,         // Client -> Server: Player's voluntary disconnection
  kPing,               // Ping packet
  kPong,               // Pong packet
  kMaxTypes            // Maximum number of packet types
};
}

#endif  // MY_PACKET_TYPES_HPP_
