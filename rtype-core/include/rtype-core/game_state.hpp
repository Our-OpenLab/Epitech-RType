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
     * If the player is already active, their connection is replaced with the new one.
     *
     * @param db_user_id The unique ID of the player in the database.
     * @param connection The TCP connection associated with the player.
     */
    void AddPlayer(int db_user_id, std::shared_ptr<network::TcpServerConnection<PacketType>> connection) {
        // Remove existing mappings if the user is already connected
        if (db_to_connection_map_.contains(db_user_id)) {
            const auto& old_connection = db_to_connection_map_[db_user_id];
          const int old_connection_id = old_connection->GetId();
            connection_to_db_map_.erase(old_connection_id);
            old_connection->Disconnect();
        }

        // Add new mappings
        const int connection_id = connection->GetId();
        db_to_connection_map_[db_user_id] = connection;
        connection_to_db_map_[connection_id] = db_user_id;
    }

    /**
     * @brief Removes a player from the active players list using their database ID.
     *
     * @param db_user_id The unique ID of the player in the database.
     */
    void RemovePlayerByDbId(int db_user_id) {
        auto it = db_to_connection_map_.find(db_user_id);
        if (it != db_to_connection_map_.end()) {
            int connection_id = it->second->GetId();
            connection_to_db_map_.erase(connection_id);
            db_to_connection_map_.erase(it);
        }
    }

    /**
     * @brief Removes a player from the active players list using their connection ID.
     *
     * @param connection_id The unique ID of the connection.
     */
    void RemovePlayerByConnectionId(int connection_id) {
        auto it = connection_to_db_map_.find(connection_id);
        if (it != connection_to_db_map_.end()) {
            int db_user_id = it->second;
            db_to_connection_map_.erase(db_user_id);
            connection_to_db_map_.erase(it);
        }
    }

    /**
     * @brief Checks if a player is active based on their database ID.
     *
     * @param db_user_id The unique ID of the player in the database.
     * @return True if the player is active, false otherwise.
     */
    [[nodiscard]] bool IsPlayerActiveByDbId(int db_user_id) const {
        return db_to_connection_map_.contains(db_user_id);
    }

    /**
     * @brief Checks if a player is active based on their connection ID.
     *
     * @param connection_id The unique ID of the connection.
     * @return True if the player is active, false otherwise.
     */
    [[nodiscard]] bool IsPlayerActiveByConnectionId(int connection_id) const {
        return connection_to_db_map_.contains(connection_id);
    }

    /**
     * @brief Retrieves the connection of an active player by their database ID.
     *
     * @param db_user_id The unique ID of the player in the database.
     * @return The shared pointer to the connection associated with the player, or nullptr if not found.
     */
    std::shared_ptr<network::TcpServerConnection<PacketType>> GetPlayerConnectionByDbId(int db_user_id) const {
        auto it = db_to_connection_map_.find(db_user_id);
        return (it != db_to_connection_map_.end()) ? it->second : nullptr;
    }

    /**
     * @brief Retrieves the database ID of an active player by their connection ID.
     *
     * @param connection_id The unique ID of the connection.
     * @return The database ID of the player, or std::nullopt if not found.
     */
    [[nodiscard]] std::optional<int> GetDbIdByConnectionId(
        const int connection_id) const {
        const auto it = connection_to_db_map_.find(connection_id);
        return (it != connection_to_db_map_.end()) ? std::optional<int>{it->second} : std::nullopt;
    }

    /**
     * @brief Retrieves all active players (for advanced use cases like broadcasting).
     *
     * @return A map of all active players with their database IDs and connections.
     */
    [[nodiscard]] const std::unordered_map<int, std::shared_ptr<network::TcpServerConnection<PacketType>>>& GetAllActivePlayers() const {
        return db_to_connection_map_;
    }

private:
    EventQueue& event_queue_; ///< Reference to the event queue for server events.

    // Maps database user IDs to connections
    std::unordered_map<int, std::shared_ptr<network::TcpServerConnection<PacketType>>> db_to_connection_map_;

    // Maps connection IDs to database user IDs
    std::unordered_map<int, int> connection_to_db_map_;
};

}  // namespace rtype

#endif  // GAME_STATE_H_
