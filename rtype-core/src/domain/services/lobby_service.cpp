#include "domain/services/lobby_service.hpp"

#include <iostream>
#include <sodium.h>

LobbyService::LobbyService(const std::shared_ptr<LobbyRepositoryInterface>& lobby_repository)
    : lobby_repository_(lobby_repository) {}

std::optional<Lobby> LobbyService::CreateLobby(
    const std::string& name,
    const std::optional<std::string>& password) const {
  std::optional<std::string> password_hash = std::nullopt;

  if (password.has_value()) {
    char hashed_password[crypto_pwhash_STRBYTES];
    if (crypto_pwhash_str(
        hashed_password,
        password->c_str(),
        password->size(),
        crypto_pwhash_OPSLIMIT_INTERACTIVE,
        crypto_pwhash_MEMLIMIT_INTERACTIVE
    ) != 0) {
      std::cerr << "[LobbyService] Failed to hash password for lobby creation." << std::endl;
      return std::nullopt;
    }
    password_hash = std::string(hashed_password);
  }

  return lobby_repository_->CreateLobby(name, password_hash);
}

std::optional<Lobby> LobbyService::GetLobbyById(const int lobby_id) const {
  auto lobby = lobby_repository_->GetLobbyById(lobby_id);

  if (!lobby.has_value()) {
    std::cerr << "[LobbyService] Lobby with ID " << lobby_id << " not found." << std::endl;
    return std::nullopt;
  }

  return lobby;
}

std::vector<Lobby> LobbyService::GetAllLobbies() const {
  auto lobbies = lobby_repository_->GetAllLobbies();
  std::cout << "[LobbyService] Retrieved " << lobbies.size() << " lobbies." << std::endl;
  return lobbies;
}

bool LobbyService::DeleteLobby(const int lobby_id) const {
  if (!lobby_repository_->DeleteLobby(lobby_id)) {
    std::cerr << "[LobbyService] Failed to delete lobby with ID " << lobby_id << "." << std::endl;
    return false;
  }
  return true;
}

bool LobbyService::CanJoinLobby(const Lobby& lobby, const std::string& password) const {
  if (!lobby.password_hash.has_value()) {
    return true;
  }

  if (crypto_pwhash_str_verify(
          lobby.password_hash->c_str(),
          password.c_str(),
          password.size()) != 0) {
    std::cerr << "[LobbyService] Password verification failed for lobby ID " << lobby.id << std::endl;
    return false;
          }

  return true;
}

std::vector<Lobby> LobbyService::GetLobbiesWithPagination(int offset, int limit, const std::string& search_term) const {
  return lobby_repository_->GetLobbiesWithPagination(offset, limit, search_term);
}

std::optional<Lobby> LobbyService::GetLobbyByIdWithValidation(const int lobby_id, const std::string& password) const {
  auto lobby = lobby_repository_->GetLobbyById(lobby_id);

  if (!lobby.has_value()) {
    std::cerr << "[LobbyService] Lobby with ID " << lobby_id << " not found." << std::endl;
    return std::nullopt;
  }

  if (!CanJoinLobby(*lobby, password)) {
    std::cerr << "[LobbyService] User is not authorized to join lobby ID " << lobby_id << std::endl;
    return std::nullopt;
  }

  return lobby;
}
