#ifndef CHAT_OVERLAY_HPP
#define CHAT_OVERLAY_HPP

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <memory>
#include <network/network_client.hpp>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/event_queue.hpp"
#include "core/my_packet_types.hpp"
#include "core/renderer.hpp"
#include "ui/button.hpp"
#include "ui/text.hpp"
#include "ui/text_box.hpp"

namespace rtype {

/**
 * @brief Structure to represent a chat message.
 */
struct ChatMessage {
  std::uint32_t sender_id;              ///< ID of the sender.
  std::string content;        ///< Message content.
  std::uint64_t message_id;   ///< Unique message ID.
  std::uint64_t timestamp;    ///< Unix timestamp of the message.
};

/**
 * @brief Overlay for in-game chat.
 *
 * Displays a list of users and allows the user to open chat windows.
 */
class ChatOverlay {
public:
  explicit ChatOverlay();

  /**
   * @brief Render the chat overlay on the screen.
   *
   * @param renderer The SDL_Renderer used for drawing.
   */
  void Render(SDL_Renderer* renderer);

  /**
   * @brief Handle input events (keyboard, mouse, etc.).
   *
   * @param event The SDL_Event to process.
   */
  void HandleInput(const SDL_Event& event);

  /**
   * @brief Handle the response for the user list request.
   */
  void HandleGetUserListResponse(const network::Packet<network::MyPacketType>& packet);

  /**
   * @brief Handle the response for private chat history request.
   */
  void HandlePrivateChatHistoryResponse(const network::Packet<network::MyPacketType>& packet);

  void HandlePrivateChatMessage(const network::Packet<network::MyPacketType>& packet);

private:
  TTF_Font* font_;                                ///< Font for rendering text.
  std::vector<std::unique_ptr<TextButton>> user_buttons_; ///< Buttons for user list.
  std::unordered_map<Button*, std::uint32_t> user_to_button_map_; ///< Map between buttons and user IDs.
  std::unique_ptr<TextBox> input_box_;            ///< Input box for user text.
  std::unique_ptr<TextButton> send_button_;           ///< Button to send messages.

  Renderer& renderer_;
  network::NetworkClient<network::MyPacketType>& network_server_;
  EventQueue<network::Packet<network::MyPacketType>>& event_queue_;

  std::optional<std::uint32_t> selected_user_id_; ///< ID of the currently selected user.
  std::vector<ChatMessage> current_chat_;         ///< Chat history with the selected user.
  Button* last_selected_button_ = nullptr;

  /**
   * @brief Initializes the UI components of the chat overlay.
   */
  void InitializeUI();

  /**
   * @brief Sets the selected user and updates the UI.
   *
   * @param user_id The ID of the user to select.
   */
  void SelectUser(std::uint32_t user_id);

  /**
   * @brief Sends the message entered by the user.
   */
  void SendMessage() const;

  /**
   * @brief Closes the chat area.
   */
  void CloseChatArea();
};

}  // namespace rtype

#endif  // CHAT_OVERLAY_HPP
