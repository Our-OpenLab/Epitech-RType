#ifndef PROTOCOL_HPP_
#define PROTOCOL_HPP_

#include <cstddef>
#include <cstdint>


namespace network::packets {

#pragma pack(push, 1)

/**
 * @brief Structure used to send a ping to the server.
 *
 * The client sends this structure to measure latency.
 */
struct PingPacket {
  std::uint32_t timestamp;  ///< Timestamp in milliseconds at the time of sending.
};

/**
 * @brief Structure used to respond to a ping.
 *
 * The server responds with this structure using the same timestamp.
 */
struct PongPacket {
  std::uint32_t timestamp;  ///< Timestamp from the original ping.
};

/**
 * @brief Structure used to assign a player to the game.
 *
 * This structure is sent to the client when a player is added to the game.
 */
struct PlayerAssign {
  float spawn_x;        ///< Initial X-coordinate of the player.
  float spawn_y;        ///< Initial Y-coordinate of the player.
  uint16_t score;       ///< Initial score of the player.
  uint8_t player_id;    ///< Unique ID of the player.
  uint8_t health;       ///< Initial health of the player.
};

/**
 * @brief Structure used to transmit the UDP port and private IP address from the client to the server.
 */
struct UdpPortPacket {
  uint16_t udp_port;       ///< The UDP port used by the client.
  char private_ip[16]{};     ///< The private IP address of the client (IPv4, exactly 15 chars, no null terminator).
};

/**
 * @brief Structure used to transmit player input data to the server.
 *
 * This structure encapsulates player actions and movement directions.
 */
struct PlayerInputPacket {
  u_int8_t player_id; ///< Unique ID of the player.
  uint16_t actions;  ///< Encodes player actions (bitmask).
  float dir_x;       ///< X-direction movement.
  float dir_y;       ///< Y-direction movement.
};

/**
 * @brief Structure used to send player updates to clients.
 */
struct UpdatePlayer {
  uint8_t player_id;  ///< Unique ID of the player.
  float x;            ///< X-coordinate of the player.
  float y;            ///< Y-coordinate of the player.
  uint16_t score;     ///< Current score of the player.
  uint8_t health;     ///< Current health of the player.
};

/**
 * @brief Structure used to send enemy updates to clients.
 */
struct UpdateProjectile {
  uint8_t projectile_id;  ///< Unique ID of the projectile.
  uint8_t owner_id;      ///< Unique ID of the player who fired the projectile.
  float x;            ///< X-coordinate of the projectile.
  float y;          ///< Y-coordinate of the projectile.
};

/**
 * @brief Structure used to send enemy updates to clients.
 */
struct UpdateEnemy {
  uint8_t enemy_id; ///< Unique ID of the enemy.
  float x;        ///< X-coordinate of the enemy.
  float y;      ///< Y-coordinate of the enemy.
};

#pragma pack(pop)

}  // namespace network::packets

#endif // PROTOCOL_HPP_
