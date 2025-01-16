#ifndef MESSAGE_HPP_
#define MESSAGE_HPP_

#include <ostream>
#include <string>
#include <optional>

/**
 * @brief Represents a message in the system.
 */
struct Message {
  std::uint64_t id;                      ///< Unique ID of the message.
  int sender_id;                         ///< ID of the sender.
  int recipient_id;                      ///< ID of the recipient
  std::string content;                   ///< Message content.
  std::uint64_t sent_at;                 ///< Timestamp of when the message was sent.

  /**
   * @brief Overloads the output operator for printing Message objects.
   *
   * @param os The output stream.
   * @param message The Message object to print.
   * @return std::ostream& The output stream for chaining.
   */
  friend std::ostream& operator<<(std::ostream& os, const Message& message) {
    os << "Message {"
       << "id: " << message.id << ", "
       << "sender_id: " << message.sender_id << ", "
       << "recipient_id: " << message.recipient_id << ", "
       << "content: \"" << message.content << "\", "
       << "sent_at: " << message.sent_at
       << "}";
    return os;
  }
};

#endif  // MESSAGE_HPP_
