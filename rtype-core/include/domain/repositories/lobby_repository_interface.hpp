#ifndef LOBBY_REPOSITORY_INTERFACE_HPP_
#define LOBBY_REPOSITORY_INTERFACE_HPP_

#include <string>
#include <optional>
#include <vector>
#include "domain/entities/lobby.hpp"

/**
 * @brief Interface for the lobby repository.
 *
 * Provides an abstraction for managing lobbies in the database.
 */
class LobbyRepositoryInterface {
public:
  virtual ~LobbyRepositoryInterface() = default;

  /**
   * @brief Creates a new lobby.
   *
   * @param name The name of the lobby.
   * @param password An optional password for the lobby.
   * @return std::optional<Lobby> The created lobby.
   */
  virtual std::optional<Lobby> CreateLobby(
      const std::string& name,
      const std::optional<std::string>& password) = 0;

  /**
   * @brief Retrieves a lobby by its ID.
   *
   * @param lobby_id The ID of the lobby.
   * @return std::optional<Lobby> The lobby, if found.
   */
  virtual std::optional<Lobby> GetLobbyById(int lobby_id) = 0;

  /**
   * @brief Retrieves all lobbies.
   *
   * @return std::vector<Lobby> A list of all lobbies.
   */
  virtual std::vector<Lobby> GetAllLobbies() = 0;

  /**
   * @brief Retrieves a list of lobbies with pagination.
   *
   * @param offset The offset for pagination.
   * @param limit The maximum number of lobbies to retrieve.
   * @param search_term An optional search term to filter lobbies.
   * @return std::vector<Lobby> A list of lobbies with pagination.
   */
  virtual std::vector<Lobby> GetLobbiesWithPagination(int offset, int limit, const std::string& search_term) = 0;

  /**
   * @brief Deletes a lobby by its ID.
   *
   * @param lobby_id The ID of the lobby to delete.
   * @return bool True if the lobby was successfully deleted, false otherwise.
   */
  virtual bool DeleteLobby(int lobby_id) = 0;

  /**
   *
   * @param id The ID of the lobby.
   * @return bool True if the operation was successful, false otherwise.
   */
  virtual bool StartGame(int id) = 0;

  /**
   *
   * @param id The ID of the lobby.
   * @return bool True if the operation was successful, false otherwise.
   */
  virtual bool EndGame(int id) = 0;

  /**
   *
   * @param id The ID of the lobby.
   * @return bool True if the game is active, false otherwise.
   */
  virtual bool IsGameActive(int id) const = 0;
};

#endif  // LOBBY_REPOSITORY_INTERFACE_HPP_
