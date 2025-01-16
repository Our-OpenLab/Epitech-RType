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

bool LobbyRepository::DeleteLobby(const int lobby_id) {
  return lobby_dao_->DeleteLobby(lobby_id);
}
