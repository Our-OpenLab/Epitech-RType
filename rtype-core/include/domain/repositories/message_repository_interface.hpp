#ifndef MESSAGE_REPOSITORY_INTERFACE_HPP_
#define MESSAGE_REPOSITORY_INTERFACE_HPP_

#include <string>
#include <optional>
#include <vector>
#include "domain/entities/message.hpp"

/**
 * @brief Interface for the message repository.
 *
 * Provides an abstraction for managing messages in the database.
 */
class MessageRepositoryInterface {
public:
  virtual ~MessageRepositoryInterface() = default;

  /**
   * @brief Inserts a new message.
   *
   * @param sender_id The ID of the sender.
   * @param recipient_id The ID of the recipient (optional for private
   * messages).
   * @param content The message content.
   * @return std::optional<Message> The created message.
   */
  virtual std::optional<Message> CreateMessage(
        int sender_id,
        int recipient_id,
        const std::string& content) = 0;

  /**
   * @brief Retrieves messages for a specific recipient or channel.
   *
   * @param user1_id The ID of the first user.
   * @param user2_id The ID of the second user.
   * @return std::vector<Message> A list of messages.
   */
  virtual std::vector<Message> GetMessages(
        int user1_id,
        int user2_id) = 0;
};

#endif  // MESSAGE_REPOSITORY_INTERFACE_HPP_
