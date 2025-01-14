#ifndef MESSAGE_SERVICE_HPP_
#define MESSAGE_SERVICE_HPP_

#include <memory>
#include <string>
#include <vector>
#include "domain/repositories/message_repository_interface.hpp"
#include "domain/entities/message.hpp"

/**
 * @brief Service class for managing message-related operations.
 *
 * Provides high-level APIs for sending and retrieving messages.
 */
class MessageService {
public:
  explicit MessageService(const std::shared_ptr<MessageRepositoryInterface>& message_repository);

  /**
   * @brief Saves a new message from a sender to a recipient.
   *
   * @param sender_id The ID of the sender.
   * @param recipient_id The ID of the recipient.
   * @param content The message content.
   * @return std::optional<Message> The saved message if successful, containing all relevant details.
   */
  [[nodiscard]] std::optional<Message> SaveMessage(
      int sender_id,
      int recipient_id,
      const std::string& content) const;

  /**
   * @brief Retrieves messages for a conversation between two users.
   *
   * @param user1_id The ID of the first user.
   * @param user2_id The ID of the second user.
   * @return std::vector<Message> A list of messages exchanged between the two users.
   */
  [[nodiscard]] std::vector<Message> GetMessages(
      int user1_id,
      int user2_id) const;

private:
  std::shared_ptr<MessageRepositoryInterface> message_repository_; ///< Repository for message data access.
};

#endif  // MESSAGE_SERVICE_HPP_
