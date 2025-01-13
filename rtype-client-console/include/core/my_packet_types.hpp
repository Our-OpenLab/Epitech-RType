#ifndef MY_PACKET_TYPES_HPP_
#define MY_PACKET_TYPES_HPP_

#include <cstdint>

namespace network {

enum class MyPacketType : uint32_t {
  // User management
  kUserRegister,          // Client -> Server: Request to register a new user
  kUserRegisterResponse,  // Server -> Client: Response to user registration
  kUserLogin,             // Client -> Server: Request to log in
  kUserLoginResponse,     // Server -> Client: Response to user login
  kUserLogout,            // Client -> Server: Request to log out
  kUserAuthenticated,     // Server -> Client: Acknowledge successful login/registration
  kUserAuthFailed,        // Server -> Client: Notify login/registration failure

  // Chat and messaging
  kPrivateMessage,        // Client -> Server: Send a private message
  kPrivateMessageResponse, // Server -> Client: Response to private message delivery
  kChannelMessage,        // Client -> Server: Send a channel message
  kChannelMessageResponse, // Server -> Client: Response to channel message delivery
  kPrivateChatMessage,    // Server -> Client: Deliver a private chat message
  kChannelChatMessage,    // Server -> Client: Deliver a channel chat message

  // Lobby management
  kCreateLobby,           // Client -> Server: Request to create a new lobby
  kCreateLobbyResponse,   // Server -> Client: Response to lobby creation request
  kJoinLobby,             // Client -> Server: Request to join a lobby
  kJoinLobbyResponse,     // Server -> Client: Response to join lobby request
  kLeaveLobby,            // Client -> Server: Request to leave the current lobby
  kLeaveLobbyResponse,    // Server -> Client: Response to leave lobby request
  kLobbyUpdate,           // Server -> Client: Notify about lobby updates (e.g., players joining/leaving)

  // Player ready state
  kPlayerReady,            // Client -> Server: Notify readiness
  kPlayerReadyResponse,    // Server -> Client: Acknowledge readiness

  // Player management
  kPlayerAssign,          // Server -> Client: Assign an ID to the player
  kPlayerInput,           // Client -> Server: Player's input data
  kPlayerJoin,            // Server -> Client: Notification of a new player joining
  kPlayerLeave,           // Server -> Client: Notification of a player leaving
  kDisconnect,            // Client -> Server: Player's voluntary disconnection

  // Ping/Pong for latency checks
  kPing,                  // Ping packet
  kPong,                  // Pong packet

  // Game state updates
  kUpdatePlayers,         // Server -> Client: Update all player data
  kUpdateEnemies,         // Server -> Client: Update all enemy data
  kRemoveEnemy,           // Server -> Client: Remove an enemy
  kUpdateProjectiles,     // Server -> Client: Updated position of a projectile
  kRemoveProjectile,      // Server -> Client: Remove a projectile

  // Miscellaneous
  kUdpPort,               // Client -> Server: Send UDP port to the server

  kMaxTypes               // Maximum number of packet types
};

}

#endif  // MY_PACKET_TYPES_HPP_
