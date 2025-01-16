#ifndef MESSAGE_DAO_HPP_
#define MESSAGE_DAO_HPP_

#include <memory>
#include <string>
#include <vector>
#include "database/database.hpp"
#include "domain/entities/message.hpp"

/**
 * @brief DAO class for interacting with the `messages` table in the database.
 */
class MessageDAO {
public:
  explicit MessageDAO(const std::shared_ptr<Database>& database) : database_(database) {}

  /**
   * @brief Inserts a new message into the database.
   *
   * @param sender_id The ID of the sender.
   * @param recipient_id The ID of the recipient.
   * @param content The message content.
   * @return std::optional<Message> The created message.
   */
  [[nodiscard]] std::optional<Message> InsertMessage(
      int sender_id,
      int recipient_id,
      const std::string& content) const;

  /**
   * @brief Retrieves all messages between two users.
   *
   * @param user1_id The ID of the first user.
   * @param user2_id The ID of the second user.
   * @return std::vector<Message> A list of messages exchanged between the two users.
   */
  [[nodiscard]] std::vector<Message> GetMessages(
      int user1_id,
      int user2_id) const;

private:
  std::shared_ptr<Database> database_;
};

#endif  // MESSAGE_DAO_HPP_
