#ifndef EVENT_TYPE_HPP_
#define EVENT_TYPE_HPP_

#include <cstddef>

namespace rtype {

/**
 * @brief Enum class defining the different types of events supported.
 */
enum class EventType : size_t {
  UserRegister = 0,   ///< Event triggered when a user registers.
  UserLogin,    ///< Event triggered when a user logs in.
  PrivateMessage,   ///< Event triggered when a private message is sent.
  ChannelMessage,   ///< Event triggered when a message is sent to a channel.
  CreateLobby,      ///< Event triggered when a lobby is created.
  JoinLobby,        ///< Event triggered when a player joins a lobby.
  LeaveLobby,       ///< Event triggered when a player leaves a lobby.
  PlayerReady,      ///< Event triggered when a player is ready.
  GetUserList,      ///< Event triggered when the list of users is requested.
  PrivateChatHistory, ///< Event triggered when the private chat history is requested.

  PlayerDied,
  PingTCP,
  PingUDP,
  UnhandledTCP,
  UnhandledUDP,
  MaxTypes  // Must always be the last entry, representing the number of event types.
};

}

#endif  // EVENT_TYPE_HPP_
