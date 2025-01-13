#ifndef USER_HPP_
#define USER_HPP_

#include <string>
#include <iostream>

/**
 * @brief Represents a user in the system.
 */
struct User {
  int id;                      ///< User ID (unique identifier in the database).
  std::string username;        ///< User's username.
  std::string password_hash;   ///< User's hashed password.

  /**
   * @brief Overloads the output operator for printing User objects.
   *
   * @param os The output stream.
   * @param user The User object to print.
   * @return std::ostream& The output stream for chaining.
   */
  friend std::ostream& operator<<(std::ostream& os, const User& user) {
    os << "User {"
       << "id: " << user.id << ", "
       << "username: \"" << user.username << "\", "
       << "password_hash: \"" << user.password_hash << "\""
       << "}";
    return os;
  }
};

#endif  // USER_HPP_
