#include "database/dao/lobby_player_dao.hpp"
#include <iostream>

bool LobbyPlayerDAO::InsertPlayerIntoLobby(const int user_id, const int lobby_id) {
    if (const auto it = lobby_to_players_.find(lobby_id);
        it != lobby_to_players_.end() && it->second.size() >= kMaxPlayersPerLobby) {
        std::cerr << "[LobbyPlayerDAO] Lobby " << lobby_id << " is full (max: "
                  << kMaxPlayersPerLobby << " players)." << std::endl;
        return false;
    }

    if (const auto player_it = player_info_.find(user_id);
        player_it != player_info_.end()) {
        const int old_lobby_id = player_it->second.lobby_id;
        lobby_to_players_[old_lobby_id].erase(user_id);
        if (lobby_to_players_[old_lobby_id].empty()) {
            lobby_to_players_.erase(old_lobby_id);
        }
    }

    player_info_[user_id] = {user_id, lobby_id, false};
    lobby_to_players_[lobby_id].insert(user_id);

    std::cout << "[LobbyPlayerDAO] Player " << user_id << " joined lobby " << lobby_id << "." << std::endl;
    return true;
}

bool LobbyPlayerDAO::RemovePlayerFromLobby(const int user_id) {
    const auto it = player_info_.find(user_id);
    if (it == player_info_.end()) {
        std::cerr << "[LobbyPlayerDAO] Player " << user_id
                  << " is not in any lobby." << std::endl;
        return false;
    }

    const int lobby_id = it->second.lobby_id;
    player_info_.erase(it);
    lobby_to_players_[lobby_id].erase(user_id);

    if (lobby_to_players_[lobby_id].empty()) {
        lobby_to_players_.erase(lobby_id);
    }

    std::cout << "[LobbyPlayerDAO] Player " << user_id << " left lobby " << lobby_id << "." << std::endl;
    return true;
}

std::optional<int> LobbyPlayerDAO::GetLobbyForPlayer(const int user_id) const {
    if (const auto it = player_info_.find(user_id); it != player_info_.end()) {
        return it->second.lobby_id;
    }
    return std::nullopt;
}

std::vector<int> LobbyPlayerDAO::GetPlayersInLobby(const int lobby_id) const {
    const auto it = lobby_to_players_.find(lobby_id);
    if (it == lobby_to_players_.end()) {
        return {};
    }

    return {it->second.begin(), it->second.end()};
}

bool LobbyPlayerDAO::SetPlayerReadyStatus(const int user_id, bool is_ready) {
    const auto it = player_info_.find(user_id);
    if (it == player_info_.end()) {
        std::cerr << "[LobbyPlayerDAO] Player " << user_id << " not found." << std::endl;
        return false;
    }

    it->second.is_ready = is_ready;
    std::cout << "[LobbyPlayerDAO] Player " << user_id << " ready status set to "
              << (is_ready ? "ready" : "not ready") << "." << std::endl;
    return true;
}

bool LobbyPlayerDAO::AreAllPlayersReady(const int lobby_id) const {
    const auto it = lobby_to_players_.find(lobby_id);
    if (it == lobby_to_players_.end()) {
        std::cerr << "[LobbyPlayerDAO] Lobby " << lobby_id << " not found." << std::endl;
        return false;
    }

    for (int user_id : it->second) {
        if (!player_info_.at(user_id).is_ready) {
            return false;
        }
    }

    return true;
}
