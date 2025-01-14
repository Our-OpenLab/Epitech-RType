#include "database/dao/message_dao.hpp"

std::optional<Message> MessageDAO::InsertMessage(
    const int sender_id, const int recipient_id, const std::string& content) const {
  const std::string query =
      "INSERT INTO messages (sender_id, recipient_id, content) "
      "VALUES ($1, $2, $3) RETURNING id, ROUND(EXTRACT(EPOCH FROM sent_at) * 1000) AS sent_at";

  if (const auto result =
          database_->ExecuteQuery(query, sender_id, recipient_id, content);
      result && !result->empty()) {
    const auto& row = result->front();
    return Message{
      .id = row["id"].as<std::uint64_t>(),
      .sender_id = sender_id,
      .recipient_id = recipient_id,
      .content = content,
      .sent_at = row["sent_at"].as<std::uint64_t>()
  };
  }

  return std::nullopt;
}

std::vector<Message> MessageDAO::GetMessages(const int user1_id, const int user2_id) const {
  const std::string query =
      "SELECT id, sender_id, recipient_id, content, ROUND(EXTRACT(EPOCH FROM sent_at) * 1000) AS sent_at "
      "FROM messages "
      "WHERE (sender_id = $1 AND recipient_id = $2) "
      "   OR (sender_id = $2 AND recipient_id = $1) "
      "ORDER BY sent_at ASC";

  std::vector<Message> messages;

  if (const auto result = database_->ExecuteQuery(query, user1_id, user2_id)) {
    for (const auto& row : *result) {
      try {
        messages.emplace_back(Message{
            row["id"].as<std::uint64_t>(),
            row["sender_id"].as<int>(),
            row["recipient_id"].as<int>(),
            row["content"].c_str(),
            row["sent_at"].as<std::uint64_t>()  // Conversion directe, comme dans InsertMessage
        });
      } catch (const std::exception& e) {
        std::cerr << "[MessageDAO][ERROR] Failed to parse message row: " << e.what() << std::endl;
      }
    }
  }

  return messages;
}
