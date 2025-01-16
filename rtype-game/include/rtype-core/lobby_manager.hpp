#ifndef RTYPE_CORE_LOBBY_MANAGER_HPP_
#define RTYPE_CORE_LOBBY_MANAGER_HPP_

#include <string>
#include <unordered_map>
#include <optional>

class LobbyManager {
public:
  LobbyManager();

  // Crée un nouveau lobby
  int CreateLobby(const std::string& name, const std::optional<std::string>& password) {
    if (lobbies_by_name_.count(name) > 0) {
      throw std::runtime_error("Lobby name already exists");
    }

    int lobby_id = next_lobby_id_++;
    lobbies_[lobby_id] = Lobby(lobby_id, name, password, max_players);
    lobbies_by_name_[name] = lobby_id;
    return lobby_id;
  }

  // Supprime un lobby
  void DeleteLobby(int lobby_id) {
    if (lobbies_.erase(lobby_id) > 0) {
      for (auto it = lobbies_by_name_.begin(); it != lobbies_by_name_.end(); ++it) {
        if (it->second == lobby_id) {
          lobbies_by_name_.erase(it);
          break;
        }
      }
    }
  }

  // Ajoute un joueur à un lobby
  bool AddPlayerToLobby(int lobby_id, const Player& player) {
    if (lobbies_.count(lobby_id) == 0) {
      return false;
    }
    return lobbies_[lobby_id].AddPlayer(player);
  }

  // Envoie un message dans un lobby
  bool SendMessageToLobby(int lobby_id, const std::string& message) {
    if (lobbies_.count(lobby_id) == 0) {
      return false;
    }
    lobbies_[lobby_id].AddMessage(message);
    return true;
  }

private:
  int next_lobby_id_{1};
  std::unordered_map<int, Lobby> lobbies_;
  std::unordered_map<std::string, int> lobbies_by_name_;
};

#endif  // RTYPE_CORE_LOBBY_MANAGER_HPP_
