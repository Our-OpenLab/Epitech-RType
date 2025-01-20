#ifndef PROTOCOL_HPP_
#define PROTOCOL_HPP_

#include <cstddef>
#include <cstdint>
#include <string>


namespace network::packets {

#pragma pack(push, 1)

/**
 * @brief Structure used to transmit login information.
 *
 * This structure limits the length of usernames and passwords to prevent
 * buffer overflows. Unused fields are filled with null characters ('\0').
 */
struct LoginPacket {
  char username[32]{};  ///< Username (max. 32 characters).
  char password[32]{};  ///< Password (max. 32 characters).
};

struct LoginResponsePacket {
  int status_code;  // Status code (e.g., 200 for success, 400 for failure, 500
                    // for server error)
};

/**
 * @brief Structure used to transmit registration information.
 *
 * This structure is used to register new users. It includes fields for
 * username, password, and email, all with fixed-length buffers.
 */
struct RegisterPacket {
  char username[32]{};  ///< Username (max. 32 characters).
  char password[32]{};  ///< Password (max. 32 characters).
  // char email[64];     ///< Email address (max. 64 characters).
};

struct RegisterResponsePacket {
  int status_code;  // Status code (e.g., 200 for success, 400 for failure, 500
                    // for server error)
};

/**
 * @brief Structure used for ping requests.
 *
 * The server and client can use this structure to measure latency.
 * It includes a timestamp field to track the round-trip time.
 */
struct PingPacket {
  std::uint32_t timestamp;  ///< Timestamp in milliseconds.
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
 * @brief Structure used for private messages between users.
 *
 * This structure is used to send private messages from one user to another.
 * It includes the recipient ID and the message content.
 */
struct PrivateMessagePacket {
  std::uint32_t recipient_id;  ///< ID of the recipient.
  char message[256]{};  ///< Message content (max. 255 characters + '\0').
};

/**
 * @brief Structure used for channel messages.
 *
 * This structure is used to send messages to a channel.
 * It includes the channel ID and the message content.
 */
struct ChannelMessagePacket {
  std::uint32_t channel_id;  ///< ID of the channel.
  char message[256]{};         ///< Message content (max. 255 characters + '\0').
};

/**
 * @brief Structure used for transmitting enriched private chat messages.
 *
 * This structure is sent by the server to clients, encapsulating all
 * necessary information about a private chat message. It includes the
 * sender and recipient IDs, the message content, a unique message ID,
 * and a timestamp.
 */
struct PrivateChatMessagePacket {
  std::uint32_t sender_id;  ///< ID of the sender. Represents the user who sent
                            ///< the message.
  std::uint32_t recipient_id;  ///< ID of the recipient. Represents the user who
                               ///< receives the message.
  char message[256]{};           ///< Message content. Limited to 255 characters.
  std::uint64_t message_id;    ///< Unique ID of the message. Used for tracking
                               ///< and displaying messages in order.
  std::uint64_t timestamp;     ///< Unix timestamp (in milliseconds) indicating
                            ///< when the message was processed by the server.
};

/**
 * @brief Structure used for transmitting enriched channel chat messages.
 *
 * This structure is sent by the server to clients, encapsulating all
 * necessary information about a channel chat message. It includes the
 * sender ID, channel ID, the message content, a unique message ID,
 * and a timestamp.
 */
struct ChannelChatMessagePacket {
  std::uint32_t sender_id;   ///< ID of the sender. Represents the user who sent
                             ///< the message.
  std::uint32_t channel_id;  ///< ID of the channel. Represents the channel
                             ///< where the message was sent.
  char message[256]{};         ///< Message content. Limited to 255 characters.
  std::uint64_t message_id;  ///< Unique ID of the message. Used for tracking
                             ///< and displaying messages in order.
  std::uint64_t timestamp;   ///< Unix timestamp (in milliseconds) indicating
                             ///< when the message was processed by the server.
};

/**
 * @brief Structure used for responding to private messages.
 *
 * This structure is sent by the server to the sender to confirm
 * whether the private message was successfully processed.
 */
struct PrivateMessageResponsePacket {
  int status_code;          ///< Status code (e.g., 200 for success, 400 for failure).
};

/**
 * @brief Structure used for responding to channel messages.
 *
 * This structure is sent by the server to the sender to confirm
 * whether the channel message was successfully processed.
 */
struct ChannelMessageResponsePacket {
  int status_code;          ///< Status code (e.g., 200 for success, 400 for failure).
};

/**
 * @brief Structure used to create a new lobby.
 */
struct CreateLobbyPacket {
  char name[32]{};      ///< Name of the lobby (max. 32 characters).
  char password[32]{};  ///< Optional password for the lobby (max. 32 characters).
};

/**
 * @brief Response for a CreateLobby request.
 */
struct CreateLobbyResponsePacket {
  int status_code;  ///< Status code (e.g., 200 for success, 400 for failure).
  int lobby_id;     ///< ID of the created lobby (if successful).
};

/**
 * @brief Structure used to join a lobby.
 */
struct JoinLobbyPacket {
  int lobby_id;       ///< ID of the lobby to join.
  char password[32]{};  ///< Optional password for the lobby (max. 32 characters).
};

/**
 * @brief Response for a JoinLobby request.
 */
struct JoinLobbyResponsePacket {
  int status_code;  ///< Status code (e.g., 200 for success, 403 for forbidden).
};

/**
 * @brief Structure used to leave a lobby.
 */
struct LeaveLobbyPacket {};

/**
 * @brief Response for a LeaveLobby request.
 */
struct LeaveLobbyResponsePacket {
  int status_code;  ///< Status code (e.g., 200 for success, 404 for not found).
};

/**
 * @brief Packet to notify all players in a lobby that a player has left.
 */
struct PlayerLeftLobbyPacket {
  int player_id;  ///< ID of the player who left.
};

/**
 * @brief Structure used to request player list in a lobby.
 */
struct GetLobbyPlayersPacket {
  int lobby_id;  ///< ID of the lobby for which players are requested.
};

/**
 * @brief Response packet containing the list of players in a lobby.
 */
struct GetLobbyPlayersResponsePacket {
  int status_code;  ///< Status code (e.g., 200 for success, 404 for lobby not found).

  /**
   * @brief Structure representing an individual player's information.
   */
  struct PlayerInfo {
    int player_id;      ///< ID of the player.
    char username[32]{};  ///< Username of the player.
    bool is_ready;    ///< Indicates if the player is ready.
  } players[];          ///< List of players in the lobby.
};

/**
 * @brief Packet to notify that a player has joined a lobby.
 */
struct PlayerJoinedLobbyPacket {
  int player_id;      ///< ID of the player who joined.
  char username[32]{};  ///< Username of the player who joined.
};

/**
 * @brief Packet to notify server about player's readiness.
 */
struct PlayerReadyPacket {
  bool is_ready;  ///< Readiness status (true = ready, false = not ready).
};

/**
 * @brief Packet to notify server about player's readiness.
 */
struct PlayerReadyPacketResponse {
  int status_code;  ///< Status code (e.g., 200 for success, 400 for failure).
};

/**
 * @brief Structure used to request a paginated list of users.
 *
 * This packet is sent by the client to request a specific range of users.
 */
struct GetUserListPacket {
  std::uint32_t offset;  ///< Offset to start fetching users from.
  std::uint32_t limit;   ///< Maximum number of users to fetch.
};

/**
 * @brief Response packet containing the list of users.
 */
struct GetUserListResponsePacket {
  int status_code;  ///< Status code (e.g., 200 for success, 400 for failure).

  /**
   * @brief Structure representing an individual user's information.
   */
  struct UserInfo {
    std::uint32_t user_id;    ///< Unique user ID.
    char username[32]{};        ///< Username (max. 32 characters).
    bool is_online;           ///< Indicates if the user is currently online.
  } users[];                  ///< Array of users.
};

/**
 * @brief Structure used to request the full history of private chat messages.
 *
 * This packet is sent by the client to request the chat history with a specific user.
 */
struct PrivateChatHistoryPacket {
  std::uint64_t user_id;  ///< ID of the user whose chat history is being requested.
};

/**
 * @brief Packet to receive the full history of private chat messages.
 */
struct PrivateChatHistoryResponsePacket {
  int status_code;  ///< Status code (e.g., 200 for success, 404 for not found).

  /**
   * @brief Structure representing a single message in the chat history.
   */
  struct MessageInfo {
    std::uint32_t sender_id;    ///< ID of the sender.
    char message[256]{};          ///< Message content (max. 255 characters).
    std::uint64_t message_id;   ///< Unique ID of the message.
    std::uint64_t timestamp;    ///< Unix timestamp (in milliseconds) of the message.
  } messages[];                 ///< Array of messages in the chat history.
};

/**
 * @brief Structure used to request a paginated list of lobbies.
 *
 * This packet is sent by the client to request a specific range of lobbies.
 */
struct GetLobbyListPacket {
  std::uint32_t offset;   ///< Offset to start fetching lobbies from.
  std::uint32_t limit;    ///< Maximum number of lobbies to fetch.
  char search_term[32]{};   ///< Optional search term for filtering (max. 32 characters).
};


/**
 * @brief Response packet containing the list of lobbies.
 */
struct GetLobbyListResponsePacket {
  int status_code;  ///< Status code (e.g., 200 for success, 404 for no lobbies found).

  /**
   * @brief Structure representing an individual lobby's information.
   */
  struct LobbyInfo {
    int lobby_id;         ///< Unique ID of the lobby.
    char name[32]{};        ///< Name of the lobby (max. 32 characters).
    bool has_password;    ///< Indicates if the lobby is password-protected.
  } lobbies[];            ///< List of lobbies.
};

/**
 * @brief Packet to notify a player about another player's readiness change.
 */
struct LobbyPlayerReadyPacket {
  int player_id;  ///< ID of the player whose state changed.
  bool is_ready;  ///< New readiness state of the player.
};

/**
 * @brief Packet to transmit game connection details to the client.
 *
 * This packet contains the necessary information for a client to connect
 * to the game server hosted in a pod, including the IP address and the list of ports.
 */
struct GameConnectionInfoPacket {
  char ip_address[64]{};  ///< IP address of the game pod (max. 64 characters, no null termination).
  int ports[16]{};        ///< Array of ports for the game connection (max. 16 ports).
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
