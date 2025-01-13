#ifndef MESSAGE_DAO_HPP_
#define MESSAGE_DAO_HPP_

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <pqxx/pqxx>

#include "database/database.hpp"
#include "domain/entities/message.hpp"

/**
 * @brief DAO class for interacting with the `messages` table in the database.
 */
class MessageDAO {
public:
  explicit MessageDAO(const std::shared_ptr<Database> &database) : database_(database) {}

  /**
   * @brief Inserts a new message into the database.
   *
   * @param sender_id The ID of the sender.
   * @param recipient_id The ID of the recipient (if private message).
   * @param channel_id The ID of the channel (if group message).
   * @param content The message content.
   * @return std::optional<Message> The created message.
   */
  [[nodiscard]] std::optional<Message> InsertMessage(
      int sender_id,
      std::optional<int> recipient_id,
      std::optional<int> channel_id,
      const std::string &content) const;

  /**
   * @brief Retrieves all messages for a specific recipient or channel.
   *
   * @param recipient_id The ID of the recipient (for private messages).
   * @param channel_id The ID of the channel (for group messages).
   * @return std::vector<Message> A list of messages matching the criteria.
   */
  [[nodiscard]] std::vector<Message> GetMessages(
      std::optional<int> recipient_id,
      std::optional<int> channel_id) const;

private:
  std::shared_ptr<Database> database_;
};

#endif  // MESSAGE_DAO_HPP_
