#include "ui/chat_overlay.hpp"

#include "core/packet_factory.hpp"
#include "core/resource_manager.hpp"
#include "core/service_locator.hpp"

namespace rtype {

#warning font_ implementation not good;

ChatOverlay::ChatOverlay()
    : font_(GlobalResourceManager::Instance().GetFont(
          "assets/fonts/Roboto-Regular.ttf")),
      renderer_(ServiceLocator::Get<Renderer>()),
      network_server_(ServiceLocator::Get<
                      network::NetworkClient<network::MyPacketType>>()) {
  if (!font_) {
    throw std::runtime_error("[ChatOverlay] Failed to retrieve font.");
  }

  InitializeUI();

  // Automatically request the user list upon creation
  RequestUserList(0, 50);  // Example: request the first 50 users
}

void ChatOverlay::InitializeUI() {
  // Initialize the input box
  input_box_ = std::make_unique<TextBox>(
      20, 450, 360, 40,
      std::make_unique<Text>(0, 0, "", font_, SDL_Color{255, 255, 255, 255}, renderer_.GetSDLRenderer()));
}

void ChatOverlay::Render(SDL_Renderer* renderer) {
  // Draw the chat background
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);  // Semi-transparent black
  SDL_Rect chat_background = {10, 10, 400, 500};
  SDL_RenderFillRect(renderer, &chat_background);

  // Render all chat messages
  for (const auto& message : messages_) {
    message->Render(renderer);
  }

  // Render the input box
  if (input_box_) input_box_->Render(renderer);
}

void ChatOverlay::HandleInput(const SDL_Event& event) {
  if (input_box_) input_box_->HandleInput(event);

  // Send message on Enter key
  if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN) {
    const std::string message = input_box_->GetContent();
    if (!message.empty()) {
      AddMessage("You: " + message);
      input_box_->SetContent("");  // Clear input after sending
    }
  }
}

void ChatOverlay::AddMessage(const std::string& message) {
  messages_.emplace_back(std::make_unique<Text>(
      20, 20 + static_cast<int>(messages_.size()) * 30,  // Vertical offset
      message, font_, SDL_Color{255, 255, 255, 255}, renderer_.GetSDLRenderer()));
}

void ChatOverlay::RequestUserList(const std::uint32_t offset,
                                  const std::uint32_t limit) const {
  auto packet = network::CreateGetUserListPacket<network::MyPacketType>(offset, limit);
  if (!packet) {
    std::cerr << "[ChatOverlay] Failed to create GetUserListPacket." << std::endl;
    return;
  }

  network_server_.SendTcp(*packet);
  std::cout << "[ChatOverlay] Sent GetUserListPacket to the server with offset = "
            << offset << ", limit = " << limit << "." << std::endl;
}

}  // namespace rtype
