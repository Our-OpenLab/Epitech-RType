#ifndef GAME_STATE_H_
#define GAME_STATE_H_

#include <unordered_map>
#include <string>
#include <memory>

#include "event_queue.hpp"
#include "network/tcp/tcp_server_connection.hpp"

namespace rtype {

template <typename PacketType>
class GameState {
public:
  explicit GameState(EventQueue& event_queue) : event_queue_(event_queue) {}
  ~GameState() = default;


  /**
   * @brief Adds a player to the active players list.
   *
   * @param user_id The unique ID of the player.
   * @param connection The TCP connection associated with the player.
   */
  void AddPlayer(int user_id, std::shared_ptr<network::TcpServerConnection<PacketType>> connection) {
    if (active_players_.contains(user_id)) {
#warning to fix .....
      active_players_[user_id]->Disconnect();
    }
    active_players_[user_id] = std::move(connection);
  }

  /**
   * @brief Removes a player from the active players list.
   *
   * @param user_id The unique ID of the player.
   */
  //void RemovePlayer(int user_id) {
  //  active_players_.erase(user_id);
  //}

  /**
   * @brief Checks if a player is active.
   *
   * @param user_id The unique ID of the player.
   * @return True if the player is active, false otherwise.
   */
  [[nodiscard]] bool IsPlayerActive(int user_id) const {
    return active_players_.contains(user_id);
  }

  /**
   * @brief Retrieves the connection of an active player.
   *
   * @param user_id The unique ID of the player.
   * @return The shared pointer to the connection associated with the player, or nullptr if not found.
   */
  std::shared_ptr<network::TcpServerConnection<PacketType>> GetPlayerConnection(int user_id) const {
    auto it = active_players_.find(user_id);
    if (it != active_players_.end()) {
      return it->second;
    }
    return nullptr;
  }


private:
  EventQueue& event_queue_;

  // Tracks active players and their connections
  std::unordered_map<int, std::shared_ptr<network::TcpServerConnection<PacketType>>> active_players_;
};

}  // namespace rtype

#endif // GAME_STATE_H_
