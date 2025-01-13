#ifndef LOBBY_PLAYER_SERVICE_HPP_
#define LOBBY_PLAYER_SERVICE_HPP_

#include <memory>
#include <optional>
#include <vector>
#include "domain/repositories/lobby_player_repository_interface.hpp"

/**
 * @brief Service class for managing player-lobby relationships.
 */
class LobbyPlayerService {
public:
  explicit LobbyPlayerService(const std::shared_ptr<LobbyPlayerRepositoryInterface>& repository);

  /**
   * @brief Adds a player to a lobby.
   *
   * @param player_id The ID of the player.
   * @param lobby_id The ID of the lobby.
   * @return bool True if the operation was successful, false otherwise.
   */
  [[nodiscard]] bool AddPlayerToLobby(int player_id, int lobby_id) const;

  /**
   * @brief Removes a player from a lobby.
   *
   * @param player_id The ID of the player.
   * @return bool True if the operation was successful, false otherwise.
   */
  [[nodiscard]] bool RemovePlayerFromLobby(int player_id) const;

  /**
   * @brief Retrieves the lobby ID for a specific player.
   *
   * @param player_id The ID of the player.
   * @return std::optional<int> The lobby ID if the player is in a lobby, or std::nullopt otherwise.
   */
  [[nodiscard]] std::optional<int> GetLobbyForPlayer(int player_id) const;

  /**
   * @brief Retrieves all players in a specific lobby.
   *
   * @param lobby_id The ID of the lobby.
   * @return std::vector<int> A list of player IDs in the lobby.
   */
  [[nodiscard]] std::vector<int> GetPlayersInLobby(int lobby_id) const;

  /**
   * @brief Sets the ready status of a player.
   *
   * @param player_id The ID of the player.
   * @param is_ready The ready status to set.
   * @return bool True if the operation was successful, false otherwise.
   */
  [[nodiscard]] bool SetPlayerReadyStatus(int player_id, bool is_ready) const;

  /**
   * @brief Checks if all players in a lobby are ready.
   *
   * @param lobby_id The ID of the lobby.
   * @return bool True if all players in the lobby are ready, false otherwise.
   */
  [[nodiscard]] bool AreAllPlayersReady(int lobby_id) const;

private:
  std::shared_ptr<LobbyPlayerRepositoryInterface> repository_; ///< Repository for managing player-lobby relationships.
};

#endif  // LOBBY_PLAYER_SERVICE_HPP_
