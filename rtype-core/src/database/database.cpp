#include "database/database.hpp"

Database::Database(const std::string& connection_string)
    : connection_(std::make_unique<pqxx::connection>(connection_string)) {
  if (!connection_->is_open()) {
    throw std::runtime_error("Failed to connect to the database.");
  }
}

std::optional<pqxx::result> Database::ExecuteQuery(const std::string& query) const {
  try {
    pqxx::work tx(*connection_);
    auto result = tx.exec(query);
    tx.commit();
    return result;
  } catch (const pqxx::sql_error& e) {
    LogSqlError(e);
  } catch (const std::exception& e) {
    LogGenericError(e);
  }
  return std::nullopt;
}

void Database::LogSqlError(const pqxx::sql_error& e) {
  std::cerr << "[Database][Error] SQL error: " << e.what() << std::endl;
  std::cerr << "[Database][Query] Query was: " << e.query() << std::endl;
}

void Database::LogGenericError(const std::exception& e) {
  std::cerr << "[Database][Error] Generic error: " << e.what() << std::endl;
}
