#ifndef DATABASE_HPP_
#define DATABASE_HPP_

#include <pqxx/pqxx>
#include <string>
#include <memory>
#include <iostream>
#include <optional>

/**
 * @brief Handles interactions with the PostgreSQL database.
 */
class Database {
public:
  /**
   * @brief Constructor to initialize the database connection.
   * @param connection_string The PostgreSQL connection string.
   */
  explicit Database(const std::string& connection_string);

  /**
   * @brief Executes a query and returns the result.
   * @param query The SQL query string to execute.
   * @return std::optional<pqxx::result> containing the query result or std::nullopt in case of an error.
   */
  [[nodiscard]] std::optional<pqxx::result> ExecuteQuery(const std::string& query) const;

  /**
   * @brief Executes a parameterized query with arguments.
   * @param query The SQL query with placeholders.
   * @param args Arguments for placeholders.
   * @return std::optional<pqxx::result> containing the query result or std::nullopt in case of an error.
   */
  template <typename... Args>
  [[nodiscard]] std::optional<pqxx::result> ExecuteQuery(const std::string& query, Args... args) {
    try {
      pqxx::work tx(*connection_);
      auto result = tx.exec(query, pqxx::params(args...));
      tx.commit();
      return result;
    } catch (const pqxx::sql_error& e) {
      LogSqlError(e);
    } catch (const std::exception& e) {
      LogGenericError(e);
    }
    return std::nullopt;
  }

private:
  std::unique_ptr<pqxx::connection> connection_;  ///< Connection to the PostgreSQL database.

  /**
   * @brief Logs SQL errors with additional context.
   * @param e The pqxx::sql_error exception.
   */
  static void LogSqlError(const pqxx::sql_error& e);

  /**
   * @brief Logs generic exceptions.
   * @param e The std::exception.
   */
  static void LogGenericError(const std::exception& e);
};

#endif  // DATABASE_HPP_
