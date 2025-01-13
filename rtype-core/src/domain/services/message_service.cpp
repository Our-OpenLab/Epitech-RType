#include "domain/services/message_service.hpp"

#include <iostream>

MessageService::MessageService(const std::shared_ptr<MessageRepositoryInterface>& message_repository)
    : message_repository_(message_repository) {}

std::optional<Message> MessageService::SaveMessage(
    const int sender_id,
    const std::optional<int> recipient_id,
    const std::optional<int> channel_id,
    const std::string& content) const {
  return message_repository_->CreateMessage(sender_id, recipient_id, channel_id, content);
}

std::vector<Message> MessageService::GetMessages(
    const std::optional<int> recipient_id,
    const std::optional<int> channel_id) const {
  return message_repository_->GetMessages(recipient_id, channel_id);
}
