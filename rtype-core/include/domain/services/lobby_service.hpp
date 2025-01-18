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
  /**
   * @brief Constructor to initialize the lobby service.
   *
   * @param lobby_repository The repository interface for lobby data access.
   */
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
   * @brief Retrieves a lobby by its ID and validates if a user can join.
   *
   * @param lobby_id The ID of the lobby.
   * @param password The password provided by the user.
   * @return std::optional<Lobby> The lobby if found and the password matches, or std::nullopt otherwise.
   */
  [[nodiscard]] std::optional<Lobby> GetLobbyByIdWithValidation(int lobby_id, const std::string& password) const;

  /**
   * @brief Checks if a user can join a lobby.
   *
   * @param lobby The lobby object to validate against.
   * @param password The password provided by the user.
   * @return bool True if the user can join, false otherwise.
   */
  [[nodiscard]] bool CanJoinLobby(const Lobby& lobby, const std::string& password) const;

  /**
   * @brief Retrieves all lobbies.
   *
   * @return std::vector<Lobby> A list of all lobbies.
   */
  [[nodiscard]] std::vector<Lobby> GetAllLobbies() const;

  /**
   * @brief Retrieves lobbies with pagination and search term.
   *
   * @param offset The offset for pagination.
   * @param limit The limit for pagination.
   * @param search_term The search term to filter lobbies.
   * @return std::vector<Lobby> A list of lobbies matching the criteria.
   */
  [[nodiscard]] std::vector<Lobby> GetLobbiesWithPagination(int offset, int limit, const std::string& search_term) const;

  /**
   * @brief Deletes a lobby by its ID.
   *
   * @param lobby_id The ID of the lobby.
   * @return bool True if the lobby was deleted, false otherwise.
   */
  bool DeleteLobby(int lobby_id) const;

  bool StartGame(const int id) const {
    return lobby_repository_->StartGame(id);
  }

  bool EndGame(const int id) const {
    return lobby_repository_->EndGame(id);
  }

  bool IsGameActive(const int id) const {
    return lobby_repository_->IsGameActive(id);
  }

private:
  std::shared_ptr<LobbyRepositoryInterface> lobby_repository_; ///< Repository for lobby data access.
};

#endif  // LOBBY_SERVICE_HPP_
