#ifndef USER_DAO_HPP_
#define USER_DAO_HPP_

#include <memory>
#include <optional>
#include <pqxx/pqxx>
#include <string>
#include <utility>

#include "database/database.hpp"
#include "domain/entities/user.hpp"

/**
 * @brief DAO class for interacting with the `users` table in the database.
 */
class UserDAO {
public:
  explicit UserDAO(const std::shared_ptr<Database> &database) : database_(database) {}

  /**
   * @brief Inserts a new user into the database.
   *
   * @param username The username of the new user.
   * @param password_hash The hashed password of the user.
   */
  [[nodiscard]] bool InsertUser(const std::string& username,
                 const std::string& password_hash) const;

  /**
   * @brief Retrieves a user by their ID.
   *
   * @param id The ID of the user.
   * @return std::optional<User> The user, if found.
   */
  [[nodiscard]] std::optional<User> GetUserById(int id) const;

  /**
   * @brief Retrieves a user by their username.
   *
   * @param username The username of the user.
   * @return std::optional<User> The user, if found.
   */
  [[nodiscard]] std::optional<User> GetUserByUsername(const std::string& username) const;

private:
  std::shared_ptr<Database> database_;
};

#endif  // USER_DAO_HPP_
