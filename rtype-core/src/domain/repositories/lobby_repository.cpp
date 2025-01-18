#include "domain/repositories/lobby_repository.hpp"

std::optional<Lobby> LobbyRepository::CreateLobby(
    const std::string& name,
    const std::optional<std::string>& password) {
  return lobby_dao_->InsertLobby(name, password);
}

std::optional<Lobby> LobbyRepository::GetLobbyById(const int lobby_id) {
  return lobby_dao_->GetLobbyById(lobby_id);
}

std::vector<Lobby> LobbyRepository::GetAllLobbies() {
  return lobby_dao_->GetAllLobbies();
}

std::vector<Lobby> LobbyRepository::GetLobbiesWithPagination(int offset, int limit, const std::string& search_term) {
  return lobby_dao_->GetLobbiesWithPagination(offset, limit, search_term);
}

bool LobbyRepository::DeleteLobby(const int lobby_id) {
  return lobby_dao_->DeleteLobby(lobby_id);
}
