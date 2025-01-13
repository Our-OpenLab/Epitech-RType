#ifndef USER_REPOSITORY_INTERFACE_HPP_
#define USER_REPOSITORY_INTERFACE_HPP_

#include <string>
#include <optional>
#include "domain/entities/user.hpp"

/**
 * @brief Interface for user repository.
 */
class UserRepositoryInterface {
public:
  virtual ~UserRepositoryInterface() = default;

  /**
   * @brief Creates a new user.
   *
   * @param username The username of the user.
   * @param password_hash The hashed password of the user.
   */
  virtual bool CreateUser(const std::string& username, const std::string& password_hash) = 0;

  /**
   * @brief Retrieves a user by ID.
   *
   * @param id The user ID.
   * @return std::optional<User> The user, if found.
   */
  virtual std::optional<User> GetUserById(int id) = 0;

  /**
   * @brief Retrieves a user by username.
   *
   * @param username The username of the user.
   * @return std::optional<User> The user, if found.
   */
  virtual std::optional<User> GetUserByUsername(const std::string& username) = 0;
};

#endif  // USER_REPOSITORY_INTERFACE_HPP_
