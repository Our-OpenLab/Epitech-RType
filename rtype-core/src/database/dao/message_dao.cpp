#include "database/dao/message_dao.hpp"

std::optional<Message> MessageDAO::InsertMessage(
    const int sender_id, const std::optional<int> recipient_id,
    const std::optional<int> channel_id,
    const std::string &content) const {
  const std::string query =
      "INSERT INTO messages (sender_id, recipient_id, channel_id, content) "
      "VALUES ($1, $2, $3, $4) RETURNING id, ROUND(EXTRACT(EPOCH FROM sent_at) * 1000) AS sent_at";

  const auto result = database_->ExecuteQuery(
      query,
      sender_id,
      recipient_id,
      channel_id,
      content);

  if (result && !result->empty()) {
    const auto &row = result->front();
    return Message{
      .id = row["id"].as<std::uint64_t>(),
      .sender_id = sender_id,
      .recipient_id = recipient_id,
      .channel_id = channel_id,
      .content = content,
      .sent_at = row["sent_at"].as<std::uint64_t>()
  };
  }

  return std::nullopt;
}


std::vector<Message> MessageDAO::GetMessages(
    const std::optional<int> recipient_id,
    const std::optional<int> channel_id) const {
  const std::string query =
      "SELECT id, sender_id, recipient_id, channel_id, content, sent_at "
      "FROM messages WHERE "
      "(recipient_id = $1 OR $1 IS NULL) AND "
      "(channel_id = $2 OR $2 IS NULL) "
      "ORDER BY sent_at ASC";

  std::vector<Message> messages;

  const auto result = database_->ExecuteQuery(
      query,
      recipient_id,
      channel_id);

  if (result) {
    for (const auto &row : *result) {
      messages.emplace_back(Message{
          row["id"].as<std::uint64_t>(),
          row["sender_id"].as<int>(),
          row["recipient_id"].is_null()
              ? std::nullopt
              : std::make_optional(row["recipient_id"].as<int>()),
          row["channel_id"].is_null()
              ? std::nullopt
              : std::make_optional(row["channel_id"].as<int>()),
          row["content"].c_str(),
          row["sent_at"].as<std::uint64_t>()});
    }
  }

  return messages;
}
