#ifndef LOBBY_PLAYER_DAO_MEMORY_HPP_
#define LOBBY_PLAYER_DAO_MEMORY_HPP_

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <optional>

#include "domain/entities/lobby_player.hpp"

/**
 * @brief DAO class for managing player-lobby relationships in memory.
 */
class LobbyPlayerDAO {
public:
  LobbyPlayerDAO() = default;

  /**
   * @brief Adds a player to a lobby.
   *
   * @param user_id The ID of the player.
   * @param lobby_id The ID of the lobby.
   * @return bool True if the player was successfully added or updated, false otherwise.
   */
  [[nodiscard]] bool InsertPlayerIntoLobby(int user_id, int lobby_id);

  /**
   * @brief Removes a player from a lobby.
   *
   * @param user_id The ID of the player.
   * @return bool True if the player was successfully removed, false otherwise.
   */
  [[nodiscard]] bool RemovePlayerFromLobby(int user_id);

  /**
   * @brief Retrieves the lobby ID for a specific player.
   *
   * @param user_id The ID of the player.
   * @return std::optional<int> The lobby ID if the player is in a lobby, or std::nullopt otherwise.
   */
  [[nodiscard]] std::optional<int> GetLobbyForPlayer(int user_id) const;

  /**
   * @brief Retrieves all players in a specific lobby.
   *
   * @param lobby_id The ID of the lobby.
   * @return std::vector<int> A list of user IDs in the lobby.
   */
  [[nodiscard]] std::vector<int> GetPlayersInLobby(int lobby_id) const;

  /**
   * @brief Retrieves a list of players in a lobby along with their ready status.
   *
   * @param lobby_id The ID of the lobby.
   * @return std::vector<std::pair<int, bool>> A list of pairs, where each pair contains a player ID and their ready status.
   */
  [[nodiscard]] std::vector<std::pair<int, bool>> GetPlayersWithStatusInLobby(int lobby_id) const;

  /**
   * @brief Sets the ready status of a player.
   *
   * @param user_id The ID of the player.
   * @param is_ready The ready status to set.
   * @return bool True if the operation was successful, false otherwise.
   */
  [[nodiscard]] bool SetPlayerReadyStatus(int user_id, bool is_ready);

  /**
   * @brief Checks if all players in a lobby are ready.
   *
   * @param lobby_id The ID of the lobby.
   * @return bool True if all players are ready, false otherwise.
   */
  [[nodiscard]] bool AreAllPlayersReady(int lobby_id) const;

private:
  static constexpr int kMaxPlayersPerLobby = 10; ///< Maximum number of players per lobby.

  std::unordered_map<int, LobbyPlayer> player_info_; ///< Maps player ID to LobbyPlayer.
  std::unordered_map<int, std::unordered_set<int>> lobby_to_players_; ///< Maps lobby ID to a set of player IDs.
};

#endif  // LOBBY_PLAYER_DAO_MEMORY_HPP_
