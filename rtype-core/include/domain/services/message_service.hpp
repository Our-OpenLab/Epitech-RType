#ifndef MESSAGE_SERVICE_HPP_
#define MESSAGE_SERVICE_HPP_

#include <memory>
#include <string>
#include <optional>
#include <vector>
#include "domain/repositories/message_repository_interface.hpp"
#include "domain/entities/message.hpp"

/**
 * @brief Service class for managing message-related operations.
 *
 * Provides high-level APIs for sending, retrieving, and managing messages.
 */
class MessageService {
public:
    explicit MessageService(const std::shared_ptr<MessageRepositoryInterface>& message_repository);

  /**
     * @brief Saves a new message from a sender to a recipient or a channel.
     *
     * This method stores the message in the database for future retrieval by
     * the recipient or members of the channel.
     *
     * @param sender_id The ID of the sender.
     * @param recipient_id The ID of the recipient (optional for private messages).
     * @param channel_id The ID of the channel (optional for group messages).
     * @param content The message content.
     * @return std::optional<Message> The saved message if successful, containing all relevant details (e.g., ID, timestamp).
     */
  [[nodiscard]] std::optional<Message> SaveMessage(
      int sender_id,
      std::optional<int> recipient_id,
      std::optional<int> channel_id,
      const std::string& content) const;

    /**
     * @brief Retrieves messages for a specific recipient or channel.
     *
     * @param recipient_id The ID of the recipient (optional for private messages).
     * @param channel_id The ID of the channel (optional for group messages).
     * @return std::vector<Message> A list of messages.
     */
    [[nodiscard]] std::vector<Message> GetMessages(
        std::optional<int> recipient_id,
        std::optional<int> channel_id) const;

private:
    std::shared_ptr<MessageRepositoryInterface> message_repository_; ///< Repository for message data access.
};

#endif  // MESSAGE_SERVICE_HPP_
