#ifndef LOBBY_SERVICE_HPP_
#define LOBBY_SERVICE_HPP_

#include <memory>
#include <string>
#include <optional>
#include <vector>
#include "domain/repositories/lobby_repository_interface.hpp"
#include "domain/entities/lobby.hpp"

/**
 * @brief Service class for managing lobby-related operations.
 *
 * Provides high-level APIs for creating, retrieving, and managing lobbies.
 */
class LobbyService {
public:
  explicit LobbyService(const std::shared_ptr<LobbyRepositoryInterface>& lobby_repository);

  /**
   * @brief Creates a new lobby.
   *
   * @param name The name of the lobby.
   * @param password The optional password for the lobby.
   * @return std::optional<Lobby> The created lobby if successful.
   */
  [[nodiscard]] std::optional<Lobby> CreateLobby(const std::string& name, const std::optional<std::string>& password) const;

  /**
   * @brief Retrieves a lobby by its ID.
   *
   * @param lobby_id The ID of the lobby.
   * @return std::optional<Lobby> The lobby if found, or std::nullopt otherwise.
   */
  [[nodiscard]] std::optional<Lobby> GetLobbyById(int lobby_id) const;

  /**
   * @brief Retrieves all lobbies.
   *
   * @return std::vector<Lobby> A list of all lobbies.
   */
  [[nodiscard]] std::vector<Lobby> GetAllLobbies() const;

  /**
   * @brief Deletes a lobby by its ID.
   *
   * @param lobby_id The ID of the lobby.
   * @return bool True if the lobby was deleted, false otherwise.
   */
  bool DeleteLobby(int lobby_id) const;

private:
  std::shared_ptr<LobbyRepositoryInterface> lobby_repository_; ///< Repository for lobby data access.
};

#endif  // LOBBY_SERVICE_HPP_
