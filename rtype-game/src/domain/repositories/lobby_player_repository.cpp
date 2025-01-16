#include "domain/repositories/lobby_player_repository.hpp"

bool LobbyPlayerRepository::AddPlayerToLobby(const int player_id, const int lobby_id) {
  return dao_->InsertPlayerIntoLobby(player_id, lobby_id);
}

bool LobbyPlayerRepository::RemovePlayerFromLobby(const int player_id) {
  return dao_->RemovePlayerFromLobby(player_id);
}

std::optional<int> LobbyPlayerRepository::GetLobbyForPlayer(const int player_id) {
  return dao_->GetLobbyForPlayer(player_id);
}

std::vector<int> LobbyPlayerRepository::GetPlayersInLobby(const int lobby_id) {
  return dao_->GetPlayersInLobby(lobby_id);
}

bool LobbyPlayerRepository::SetPlayerReadyStatus(const int player_id,
                                                 const bool is_ready) {
  return dao_->SetPlayerReadyStatus(player_id, is_ready);
}

bool LobbyPlayerRepository::AreAllPlayersReady(
    const int lobby_id) const {
  return dao_->AreAllPlayersReady(lobby_id);
}