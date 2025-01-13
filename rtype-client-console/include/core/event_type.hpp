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
  RegisterSuccess,
  RegisterFailed,

  // Authentication
  LoginRequest,
  LoginSuccess,
  LoginFailed,

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

  // Must always be the last entry
  MaxTypes
};

}

#endif  // RTYPE_CLIENT_CONNECTION_EVENT_TYPE_HPP_
