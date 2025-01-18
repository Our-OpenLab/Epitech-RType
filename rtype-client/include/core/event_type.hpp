#ifndef RTYPE_CLIENT_CONNECTION_EVENT_TYPE_HPP_
#define RTYPE_CLIENT_CONNECTION_EVENT_TYPE_HPP_

#include <cstddef>

namespace rtype {

/**
 * @brief Enum class defining connection-related events for the client.
 */
enum class EventType : size_t {
  // Registration
  RegisterRequest,
  RegisterResponse,

  // Authentication
  LoginRequest,
  LoginResponse,

  // Logout
  LogoutRequest,
  LogoutSuccess,

  // Session management
  ReconnectRequest,
  ReconnectSuccess,
  SessionExpired,

  // Network errors
  ConnectionLost,
  ServerUnreachable,
  UnauthorizedAccess,

  GameConnectionInfo, // Information about the game server connection

  // User management
  GetUserListRequest,   // Request to fetch the list of users
  GetUserListResponse,  // Response containing the list of users

  PlayerReadyResponse,
  LobbyPlayerReady,

  PrivateChatHistoryResponse,
  PrivateChatMessage,
  CreateLobbyResponse,
  LeaveLobbyResponse,
  PlayerJoinedLobby,
  PlayerLeftLobby,
  GetLobbyPlayersResponse,
  LobbiesResponse,
  JoinLobbyResponse,
  GetLobbyListResponse,
  // Must always be the last entry
  MaxTypes
};

}  // namespace rtype

#endif  // RTYPE_CLIENT_CONNECTION_EVENT_TYPE_HPP_
