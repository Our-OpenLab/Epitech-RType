#ifndef MESSAGE_REPOSITORY_HPP_
#define MESSAGE_REPOSITORY_HPP_

#include <memory>
#include <optional>

#include "message_repository_interface.hpp"
#include "database/dao/message_dao.hpp"

/**
 * @brief Implementation of the MessageRepositoryInterface.
 *
 * Manages message data by interacting with the database through the DAO.
 */
class MessageRepository final : public MessageRepositoryInterface {
public:
  explicit MessageRepository(const std::shared_ptr<MessageDAO>& message_dao)
      : message_dao_(message_dao) {}

  std::optional<Message> CreateMessage(
        int sender_id,
        int recipient_id,
        const std::string& content) override;

  std::vector<Message> GetMessages(
        int user1_id,
        int user2_id) override;

private:
  std::shared_ptr<MessageDAO> message_dao_; ///< DAO for interacting with the database.
};

#endif  // MESSAGE_REPOSITORY_HPP_
