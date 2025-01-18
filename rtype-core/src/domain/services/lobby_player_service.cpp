#include "domain/services/lobby_player_service.hpp"

LobbyPlayerService::LobbyPlayerService(const std::shared_ptr<LobbyPlayerRepositoryInterface>& repository)
    : repository_(repository) {}

bool LobbyPlayerService::AddPlayerToLobby(const int player_id, const int lobby_id) const {
  return repository_->AddPlayerToLobby(player_id, lobby_id);
}

bool LobbyPlayerService::RemovePlayerFromLobby(const int player_id) const {
  return repository_->RemovePlayerFromLobby(player_id);
}

std::optional<int> LobbyPlayerService::GetLobbyForPlayer(const int player_id) const {
  return repository_->GetLobbyForPlayer(player_id);
}

std::vector<int> LobbyPlayerService::GetPlayersInLobby(const int lobby_id) const {
  return repository_->GetPlayersInLobby(lobby_id);
}

std::vector<std::pair<int, bool>> LobbyPlayerService::GetPlayersWithStatusInLobby(const int lobby_id) const {
  return repository_->GetPlayersWithStatusInLobby(lobby_id);
}

bool LobbyPlayerService::SetPlayerReadyStatus(const int player_id,
                                              const bool is_ready) const {
  return repository_->SetPlayerReadyStatus(player_id, is_ready);
}

bool LobbyPlayerService::AreAllPlayersReady(const int lobby_id) const {
  return repository_->AreAllPlayersReady(lobby_id);
}