#include "domain/repositories/message_repository.hpp"

std::optional<Message> MessageRepository::CreateMessage(
    const int sender_id,
    const std::optional<int> recipient_id,
    const std::optional<int> channel_id,
    const std::string& content) {
  return message_dao_->InsertMessage(sender_id, recipient_id, channel_id, content);
}

std::vector<Message> MessageRepository::GetMessages(
    const std::optional<int> recipient_id,
    const std::optional<int> channel_id) {
  return message_dao_->GetMessages(recipient_id, channel_id);
}
