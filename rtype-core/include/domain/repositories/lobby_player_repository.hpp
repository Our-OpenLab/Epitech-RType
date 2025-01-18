#ifndef LOBBY_PLAYER_REPOSITORY_HPP_
#define LOBBY_PLAYER_REPOSITORY_HPP_

#include <memory>
#include <optional>
#include <vector>

#include "lobby_player_repository_interface.hpp"
#include "database/dao/lobby_player_dao.hpp"

/**
 * @brief Repository for managing player-lobby relationships.
 */
class LobbyPlayerRepository : public LobbyPlayerRepositoryInterface {
public:
  explicit LobbyPlayerRepository(const std::shared_ptr<LobbyPlayerDAO>& dao) : dao_(dao) {}

  /**
   * @brief Adds a player to a lobby.
   *
   * @param player_id The ID of the player.
   * @param lobby_id The ID of the lobby.
   * @return bool True if the operation was successful, false otherwise.
   */
  bool AddPlayerToLobby(int player_id, int lobby_id) override;

  /**
   * @brief Removes a player from a lobby.
   *
   * @param player_id The ID of the player.
   * @return bool True if the operation was successful, false otherwise.
   */
  bool RemovePlayerFromLobby(int player_id) override;

  /**
   * @brief Retrieves the lobby ID for a specific player.
   *
   * @param player_id The ID of the player.
   * @return std::optional<int> The lobby ID if the player is in a lobby, or std::nullopt otherwise.
   */
  std::optional<int> GetLobbyForPlayer(int player_id) override;

  /**
   * @brief Retrieves all players in a specific lobby.
   *
   * @param lobby_id The ID of the lobby.
   * @return std::vector<int> A list of player IDs in the lobby.
   */
  std::vector<int> GetPlayersInLobby(int lobby_id) override;

  /**
   * @brief Retrieves a list of players in a lobby along with their ready
   * status.
   *
   * @param lobby_id The ID of the lobby.
   * @return std::vector<std::pair<int, bool>> A list of pairs, where each pair
   * contains a player ID and their ready status.
   */
  std::vector<std::pair<int, bool>> GetPlayersWithStatusInLobby(
    int lobby_id) override;

  /**
   * @brief Sets the ready status of a player.
   *
   * @param player_id The ID of the player.
   * @param is_ready The ready status to set.
   * @return bool True if the operation was successful, false otherwise.
   */
  bool SetPlayerReadyStatus(int player_id, bool is_ready) override;

  /**
   * @brief Checks if all players in a lobby are ready.
   *
   * @param lobby_id The ID of the lobby.
   * @return bool True if all players are ready, false otherwise.
   */
  bool AreAllPlayersReady(int lobby_id) const override;

private:
  std::shared_ptr<LobbyPlayerDAO> dao_; ///< DAO for interacting with the database.
};

#endif  // LOBBY_PLAYER_REPOSITORY_HPP_
