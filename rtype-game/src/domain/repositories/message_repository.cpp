#include "domain/repositories/message_repository.hpp"

std::optional<Message> MessageRepository::CreateMessage(
    const int sender_id,
    const int recipient_id,
    const std::string& content) {
  return message_dao_->InsertMessage(sender_id, recipient_id, content);
}

std::vector<Message> MessageRepository::GetMessages(const int user1_id,
                                                    const int user2_id) {
  return message_dao_->GetMessages(user1_id, user2_id);
}
