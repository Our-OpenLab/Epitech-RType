#ifndef PROTOCOL_HPP_
#define PROTOCOL_HPP_

#include <cstddef>
#include <cstdint>

namespace network::packets {
/**
 * @brief Structure used to transmit login information.
 *
 * This structure limits the length of usernames and passwords to prevent
 * buffer overflows. Unused fields are filled with null characters ('\0').
 */
struct LoginPacket {
  char username[32];  ///< Username (max. 32 characters).
  char password[32];  ///< Password (max. 32 characters).
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
  char username[32];  ///< Username (max. 32 characters).
  char password[32];  ///< Password (max. 32 characters).
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
 * @brief Structure used for private messages between users.
 *
 * This structure is used to send private messages from one user to another.
 * It includes the recipient ID and the message content.
 */
struct PrivateMessagePacket {
  std::uint32_t recipient_id;  ///< ID of the recipient.
  char message[256];  ///< Message content (max. 255 characters + '\0').
};

/**
 * @brief Structure used for channel messages.
 *
 * This structure is used to send messages to a channel.
 * It includes the channel ID and the message content.
 */
struct ChannelMessagePacket {
  std::uint32_t channel_id;  ///< ID of the channel.
  char message[256];         ///< Message content (max. 255 characters + '\0').
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
  char message[256];           ///< Message content. Limited to 255 characters.
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
  char message[256];         ///< Message content. Limited to 255 characters.
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
  char name[32];      ///< Name of the lobby (max. 32 characters).
  char password[32];  ///< Optional password for the lobby (max. 32 characters).
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
  char password[32];  ///< Optional password for the lobby (max. 32 characters).
};

/**
 * @brief Response for a JoinLobby request.
 */
struct JoinLobbyResponsePacket {
  int status_code;  ///< Status code (e.g., 200 for success, 403 for forbidden).
  struct PlayerInfo {
    int player_id;      ///< ID of the player.
    char username[32];  ///< Username of the player.
  } players[];          ///< List of players in the lobby.
};

/**
 * @brief Structure used to leave a lobby.
 */
struct LeaveLobbyPacket {
  int lobby_id;  ///< ID of the lobby to leave.
};

/**
 * @brief Response for a LeaveLobby request.
 */
struct LeaveLobbyResponsePacket {
  int status_code;  ///< Status code (e.g., 200 for success, 404 for not found).
};

/**
 * @brief Packet to notify that a player has joined a lobby.
 */
struct LobbyPlayerJoinedPacket {
  int lobby_id;       ///< ID of the lobby.
  int player_id;      ///< ID of the player who joined.
  char username[32];  ///< Username of the player who joined.
};

/**
 * @brief Packet to notify that a player has left a lobby.
 */
struct LobbyPlayerLeftPacket {
  int lobby_id;   ///< ID of the lobby.
  int player_id;  ///< ID of the player who left.
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

}  // namespace network::packets

#endif // PROTOCOL_HPP_
