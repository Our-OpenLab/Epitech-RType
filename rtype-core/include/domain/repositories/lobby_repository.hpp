#ifndef LOBBY_REPOSITORY_HPP_
#define LOBBY_REPOSITORY_HPP_

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "lobby_repository_interface.hpp"
#include "database/dao/lobby_dao.hpp"

/**
 * @brief Implementation of the LobbyRepositoryInterface.
 *
 * Manages lobby data by interacting with the database through the DAO.
 */
class LobbyRepository final : public LobbyRepositoryInterface {
public:
  explicit LobbyRepository(const std::shared_ptr<LobbyDAO>& lobby_dao)
      : lobby_dao_(lobby_dao) {}

  std::optional<Lobby> CreateLobby(const std::string& name, const std::optional<std::string>& password) override;

  std::optional<Lobby> GetLobbyById(int lobby_id) override;

  std::vector<Lobby> GetAllLobbies() override;

  std::vector<Lobby> GetLobbiesWithPagination(int offset, int limit, const std::string& search_term) override;

  bool DeleteLobby(int lobby_id) override;

  bool StartGame(const int id) override {
    return lobby_dao_->StartGame(id);
  }

  bool EndGame(const int id) override {
    return lobby_dao_->EndGame(id);
  }

  bool IsGameActive(const int id) const override {
    return lobby_dao_->IsGameActive(id);
  }

private:
  std::shared_ptr<LobbyDAO> lobby_dao_; ///< DAO for interacting with the database.
};

#endif  // LOBBY_REPOSITORY_HPP_
