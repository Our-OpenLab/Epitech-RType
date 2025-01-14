#ifndef USER_REPOSITORY_HPP_
#define USER_REPOSITORY_HPP_

#include <memory>

#include "user_repository_interface.hpp"
#include "database/dao/user_dao.hpp"

/**
 * @brief Implementation of the user repository interface.
 */
class UserRepository final : public UserRepositoryInterface {
public:
  /**
   * @brief Constructor to initialize the repository with a DAO.
   *
   * @param user_dao The UserDAO instance for database interactions.
   */
  explicit UserRepository(const std::shared_ptr<UserDAO> &user_dao);

  bool CreateUser(const std::string& username, const std::string& password_hash) override;
  std::optional<User> GetUserById(int id) override;
  std::optional<User> GetUserByUsername(const std::string& username) override;
  [[nodiscard]] std::vector<User> GetUsers(std::uint32_t offset, std::uint32_t limit) const override;

private:
  std::shared_ptr<UserDAO> user_dao_;  ///< Data Access Object for user database operations.
};

#endif  // USER_REPOSITORY_HPP_
