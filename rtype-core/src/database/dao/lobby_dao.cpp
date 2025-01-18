#include "database/dao/lobby_dao.hpp"

std::optional<Lobby> LobbyDAO::InsertLobby(const std::string& name, const std::optional<std::string>& password_hash) {
  for (const auto& [id, lobby] : lobbies_) {
    if (lobby.name == name) {
      return std::nullopt;
    }
  }

  Lobby new_lobby{next_lobby_id_++, name, password_hash};
  lobbies_[new_lobby.id] = new_lobby;
  return new_lobby;
}

std::optional<Lobby> LobbyDAO::GetLobbyById(const int id) const {
  if (const auto it = lobbies_.find(id); it != lobbies_.end()) {
    return it->second;
  }
  return std::nullopt;
}

std::vector<Lobby> LobbyDAO::GetAllLobbies() const {
  std::vector<Lobby> result;
  result.reserve(lobbies_.size());
  for (const auto& [id, lobby] : lobbies_) {
    result.push_back(lobby);
  }
  return result;
}


std::vector<Lobby> LobbyDAO::GetLobbiesWithPagination(
    const int offset, const int limit, const std::string& search_term) const {
  std::vector<Lobby> result;
  int skipped = 0;

  for (const auto& [id, lobby] : lobbies_) {
    if (!search_term.empty() && lobby.name.find(search_term) == std::string::npos) {
      continue;
    }

    if (skipped < offset) {
      ++skipped;
      continue;
    }

    if (static_cast<int>(result.size()) < limit) {
      result.push_back(lobby);
    } else {
      break;
    }
  }

  return result;
}


bool LobbyDAO::DeleteLobby(const int id) {
  return lobbies_.erase(id) > 0;
}
