#ifndef CHAT_OVERLAY_HPP
#define CHAT_OVERLAY_HPP

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <memory>
#include <network/network_client.hpp>
#include <string>
#include <vector>

#include "core/my_packet_types.hpp"
#include "core/renderer.hpp"
#include "ui/text.hpp"
#include "ui/text_box.hpp"

namespace rtype {

/**
 * @brief Overlay for in-game chat.
 *
 * Displays a list of messages and allows the user to input text.
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
   * @brief Add a message to the chat overlay.
   *
   * @param message The message to add.
   */
  void AddMessage(const std::string& message);

  /**
   * @brief Request the user list from the server.
   */
  void RequestUserList(std::uint32_t offset, std::uint32_t limit) const;

private:
  TTF_Font* font_;                            ///< Font for rendering text.
  std::vector<std::unique_ptr<Text>> messages_; ///< List of messages in the chat.
  std::unique_ptr<TextBox> input_box_;        ///< Input box for user text.

  Renderer& renderer_;
  network::NetworkClient<network::MyPacketType>& network_server_;

  /**
   * @brief Initializes the UI components of the chat overlay.
   */
  void InitializeUI();
};

}  // namespace rtype

#endif  // CHAT_OVERLAY_HPP
