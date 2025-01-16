#ifndef LOBBY_HPP_
#define LOBBY_HPP_

#include <ostream>
#include <string>
#include <optional>

/**
 * @brief Represents a lobby in the system.
 */
struct Lobby {
  int id;                                ///< Unique ID of the lobby.
  std::string name;                      ///< Name of the lobby.
  std::optional<std::string> password_hash;   ///< Optional password hash for the lobby.

  /**
   * @brief Overloads the output operator for printing Lobby objects.
   *
   * @param os The output stream.
   * @param lobby The Lobby object to print.
   * @return std::ostream& The output stream for chaining.
   */
  friend std::ostream& operator<<(std::ostream& os, const Lobby& lobby) {
    os << "Lobby {"
       << "id: " << lobby.id << ", "
       << "name: \"" << lobby.name << "\", "
       << "}";
    return os;
  }
};

#endif  // LOBBY_HPP_
