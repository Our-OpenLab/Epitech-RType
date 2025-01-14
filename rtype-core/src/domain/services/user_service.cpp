#include "domain/services/user_service.hpp"

#include <iostream>
#include <sodium.h>

UserService::UserService(const std::shared_ptr<UserRepositoryInterface> &user_repository)
    : user_repository_(user_repository) {}

bool UserService::RegisterUser(const std::string& username, const std::string& password) const {
  char password_hash[crypto_pwhash_STRBYTES];

  if (crypto_pwhash_str(
    password_hash,
    password.c_str(),
    password.size(),
    crypto_pwhash_OPSLIMIT_INTERACTIVE,
    crypto_pwhash_MEMLIMIT_INTERACTIVE
  ) != 0) {
    std::cerr << "[UserService] Failed to hash password." << std::endl;
    return false;
  }
  return user_repository_->CreateUser(username, password_hash);
}

std::optional<User> UserService::GetUserProfile(const int user_id) const {
  auto user = user_repository_->GetUserById(user_id);

  if (!user.has_value()) {
    std::cerr << "[UserService] User with ID " << user_id << " not found.\n";
    return std::nullopt;
  }

  return user;
}

std::optional<int> UserService::AuthenticateUser(const std::string& username, const std::string& password) const {
  const auto user = user_repository_->GetUserByUsername(username);
  if (!user.has_value()) {
    return std::nullopt;
  }

  if (crypto_pwhash_str_verify(user->password_hash.c_str(), password.c_str(), password.size()) != 0) {
    return std::nullopt;
  }

  return user->id;
}

std::vector<User> UserService::GetUsers(std::uint32_t offset, std::uint32_t limit) const {
  return user_repository_->GetUsers(offset, limit);
}
