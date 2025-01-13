#ifndef LOBBY_PLAYER_REPOSITORY_INTERFACE_HPP_
#define LOBBY_PLAYER_REPOSITORY_INTERFACE_HPP_

#include <optional>
#include <vector>

/**
 * @brief Interface for managing player-lobby associations.
 *
 * Provides an abstraction for operations on the `lobby_players` table.
 */
class LobbyPlayerRepositoryInterface {
public:
  virtual ~LobbyPlayerRepositoryInterface() = default;

  /**
   * @brief Adds a player to a lobby.
   *
   * @param player_id The ID of the player.
   * @param lobby_id The ID of the lobby.
   * @return bool True if the player was successfully added, false otherwise.
   */
  virtual bool AddPlayerToLobby(int player_id, int lobby_id) = 0;

  /**
   * @brief Removes a player from a lobby.
   *
   * @param player_id The ID of the player.
   * @return bool True if the player was successfully removed, false otherwise.
   */
  virtual bool RemovePlayerFromLobby(int player_id) = 0;

  /**
   * @brief Retrieves the lobby ID of a specific player.
   *
   * @param player_id The ID of the player.
   * @return std::optional<int> The ID of the lobby the player is in, or std::nullopt if not in any lobby.
   */
  virtual std::optional<int> GetLobbyForPlayer(int player_id) = 0;

  /**
   * @brief Retrieves all players in a specific lobby.
   *
   * @param lobby_id The ID of the lobby.
   * @return std::vector<int> A list of player IDs in the lobby.
   */
  virtual std::vector<int> GetPlayersInLobby(int lobby_id) = 0;

  /**
   * @brief Sets the ready status of a player.
   *
   * @param player_id The ID of the player.
   * @param is_ready The ready status to set.
   * @return bool True if the operation was successful, false otherwise.
   */
  virtual bool SetPlayerReadyStatus(int player_id, bool is_ready) = 0;

  /**
   * @brief Checks if all players in a lobby are ready.
   *
   * @param lobby_id The ID of the lobby.
   * @return bool True if all players are ready, false otherwise.
   */
  virtual bool AreAllPlayersReady(int lobby_id) const = 0;
};

#endif  // LOBBY_PLAYER_REPOSITORY_INTERFACE_HPP_
