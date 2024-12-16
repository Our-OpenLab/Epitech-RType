#ifndef MY_PACKET_TYPES_HPP_
#define MY_PACKET_TYPES_HPP_

#include <cstdint>

namespace network {
enum class MyPacketType : uint32_t {
  kPlayerAssign,       // Server -> Client: Assign an ID to the player
  kPlayerInput,        // Client -> Server: Player's input data
  kPlayerJoin,         // Server -> Client: Notification of a new player joining
  kPlayerLeave,        // Server -> Client: Notification of a player leaving
  kDisconnect,         // Client -> Server: Player's voluntary disconnection
  kPing,               // Ping packet
  kPong,               // Pong packet

  kUpdatePlayers,      // Server -> Client: Update all player data
  kUpdateEnemies,      // Server -> Client: Update all enemy data
  kRemoveEnemy,        // Server -> Client: Remove an enemy
  kUpdateProjectiles,   // Server -> Client: Updated position of a projectile
  kRemoveProjectile,   // Server -> Client: Remove a projectile

  kUdpPort,            // Client -> Server: Send UDP port to the server

  kMaxTypes            // Maximum number of packet types
};
}

#endif  // MY_PACKET_TYPES_HPP_
