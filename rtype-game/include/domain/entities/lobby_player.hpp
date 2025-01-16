#ifndef LOBBY_PLAYER_HPP_
#define LOBBY_PLAYER_HPP_

#include <ostream>
#include <cstdint>

/**
 * @brief Represents a user in a lobby.
 */
struct LobbyPlayer {
  int user_id;                 ///< ID of the user.
  int lobby_id;                ///< ID of the lobby.
  bool is_ready;               ///< Ready status of the player.

  /**
   * @brief Overloads the output operator for printing LobbyPlayer objects.
   *
   * @param os The output stream.
   * @param player The LobbyPlayer object to print.
   * @return std::ostream& The output stream for chaining.
   */
  friend std::ostream& operator<<(std::ostream& os, const LobbyPlayer& player) {
    os << "LobbyPlayer {"
       << "user_id: " << player.user_id << ", "
       << "lobby_id: " << player.lobby_id << ", "
       << "is_ready: " << (player.is_ready ? "true" : "false") << "}";
    return os;
  }
};

#endif  // LOBBY_PLAYER_HPP_
