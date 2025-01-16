#include "database/dao/user_dao.hpp"

bool UserDAO::InsertUser(const std::string& username, const std::string& password_hash) const {
  const std::string query = "INSERT INTO users (username, password_hash) VALUES ($1, $2)";

  if (const auto result = database_->ExecuteQuery(query, username, password_hash); result) {
    return true;
  }

  return false;
}

std::optional<User> UserDAO::GetUserById(const int id) const {
  const std::string query = "SELECT id, username, password_hash FROM users WHERE id = $1";
  if (const auto result = database_->ExecuteQuery(query, id);
      result && !result->empty()) {
    const pqxx::row row = result->front();
    return User{
      row["id"].as<int>(),
      row["username"].c_str(),
      row["password_hash"].c_str()
    };
  }
  return std::nullopt;
}

std::optional<User> UserDAO::GetUserByUsername(const std::string& username) const {
  const std::string query = "SELECT id, username, password_hash FROM users WHERE username = $1";
  if (const auto result = database_->ExecuteQuery(query, username);
      result && !result->empty()) {
    const pqxx::row row = result->front();
    return User{
      row["id"].as<int>(),
      row["username"].c_str(),
      row["password_hash"].c_str()
    };
  }
  return std::nullopt;
}

std::vector<User> UserDAO::GetUsers(const std::uint32_t offset,
                                    const std::uint32_t limit) const {
  const std::string query = R"(
    SELECT id, username
    FROM users
    ORDER BY id
    LIMIT $1 OFFSET $2
  )";

  const auto result = database_->ExecuteQuery(query, limit, offset);
  std::vector<User> users;

  if (result) {
    for (const auto& row : *result) {
      users.emplace_back(User{
        row["id"].as<int>(),
        row["username"].c_str(),
        ""
      });
    }
  }

  return users;
}
