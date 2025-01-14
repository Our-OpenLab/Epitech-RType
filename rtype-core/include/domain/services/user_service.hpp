#ifndef USER_SERVICE_HPP_
#define USER_SERVICE_HPP_

#include <string>
#include <memory>

#include "domain/repositories/user_repository_interface.hpp"

/**
 * @brief Service class for managing user-related operations.
 *
 * Provides a high-level API for user registration, authentication,
 * and other business logic related to users.
 */
class UserService {
public:
  /**
   * @brief Constructor to initialize the service with a user repository.
   *
   * @param user_repository The UserRepositoryInterface instance for interacting with the data layer.
   */
  explicit UserService(const std::shared_ptr<UserRepositoryInterface> &user_repository);

  /**
   * @brief Registers a new user with a username and a hashed password.
   *
   * @param username The username of the user.
   * @param password The password of the user.
   * @return true if registration is successful, false otherwise.
   */
  [[nodiscard]] bool RegisterUser(const std::string& username, const std::string& password) const;

  /**
 * @brief Retrieves the profile of a user by ID.
 * @param user_id The ID of the user.
 * @return std::optional<User> containing the user's profile data if found, or std::nullopt if not.
 */
  [[nodiscard]] std::optional<User> GetUserProfile(int user_id) const;

  /**
   * @brief Authenticates a user with a username and a hashed password.
   *
   * @param username The username of the user.
   * @param password The password of the user.
   * @return The user ID if authentication is successful, std::nullopt otherwise.
   */
  [[nodiscard]] std::optional<int> AuthenticateUser(const std::string& username, const std::string& password) const;

  /**
   * @brief Retrieves a paginated list of users.
   *
   * @param offset The starting index for pagination.
   * @param limit The maximum number of users to retrieve.
   * @return A vector of User objects.
   */
  [[nodiscard]] std::vector<User> GetUsers(std::uint32_t offset, std::uint32_t limit) const;

private:
  std::shared_ptr<UserRepositoryInterface> user_repository_; ///< Repository for user data access.
};

#endif  // USER_SERVICE_HPP_
