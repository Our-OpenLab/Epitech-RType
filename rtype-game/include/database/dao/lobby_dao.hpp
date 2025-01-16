#ifndef LOBBY_DAO_MEMORY_HPP_
#define LOBBY_DAO_MEMORY_HPP_

#include <unordered_map>
#include <optional>
#include <vector>
#include <string>
#include "domain/entities/lobby.hpp"

/**
 * @brief DAO for managing lobbies in memory.
 *
 * This class assumes it operates in a single-threaded context or that external synchronization is handled.
 */
class LobbyDAO {
public:
  LobbyDAO() = default;

  /**
   * @brief Inserts a new lobby into memory.
   *
   * @param name The name of the lobby.
   * @param password_hash Optional password hash for the lobby.
   * @return std::optional<Lobby> The created lobby, or nullopt if the name is already taken.
   */
  std::optional<Lobby> InsertLobby(const std::string& name, const std::optional<std::string>& password_hash);

  /**
   * @brief Retrieves a lobby by its ID.
   *
   * @param id The ID of the lobby.
   * @return std::optional<Lobby> The lobby if found, or nullopt otherwise.
   */
  [[nodiscard]] std::optional<Lobby> GetLobbyById(int id) const;

  /**
   * @brief Retrieves all lobbies in memory.
   *
   * @return std::vector<Lobby> A list of all lobbies.
   */
  [[nodiscard]] std::vector<Lobby> GetAllLobbies() const;

  /**
   * @brief Deletes a lobby by its ID.
   *
   * @param id The ID of the lobby to delete.
   * @return bool True if the lobby was deleted, false otherwise.
   */
  bool DeleteLobby(int id);

private:
  int next_lobby_id_{1}; ///< Auto-incrementing ID for new lobbies.
  std::unordered_map<int, Lobby> lobbies_; ///< Map of lobby ID to Lobby objects.
};

#endif  // LOBBY_DAO_MEMORY_HPP_
