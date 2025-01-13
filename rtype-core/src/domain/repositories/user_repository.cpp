#include "domain/repositories/user_repository.hpp"

UserRepository::UserRepository(const std::shared_ptr<UserDAO> &user_dao)
    : user_dao_(user_dao) {}

bool UserRepository::CreateUser(const std::string& username, const std::string& password_hash) {
  if (user_dao_->GetUserByUsername(username).has_value()) {
    return false;
  }

  return user_dao_->InsertUser(username, password_hash);
}

std::optional<User> UserRepository::GetUserById(const int id) {
  return user_dao_->GetUserById(id);
}

std::optional<User> UserRepository::GetUserByUsername(const std::string& username) {
  return user_dao_->GetUserByUsername(username);
}
