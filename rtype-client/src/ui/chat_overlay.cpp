#include "ui/chat_overlay.hpp"

#include "core/packet_factory.hpp"
#include "core/resource_manager.hpp"
#include "core/service_locator.hpp"

namespace rtype {

ChatOverlay::ChatOverlay()
    : font_(GlobalResourceManager::Instance().GetFont("assets/fonts/Roboto-Regular.ttf")),
      renderer_(ServiceLocator::Get<Renderer>("renderer")),
      network_server_(ServiceLocator::Get<network::NetworkClient<network::MyPacketType>>("network_server")),
      event_queue_(ServiceLocator::Get<EventQueue<network::Packet<network::MyPacketType>>>("event_queue")) {
  if (!font_) {
    throw std::runtime_error("[ChatOverlay] Failed to retrieve font.");
  }

  InitializeUI();

  event_queue_.Subscribe(EventType::GetUserListResponse, [this](const network::Packet<network::MyPacketType>& packet) {
    HandleGetUserListResponse(packet);
  });

  event_queue_.Subscribe(EventType::PrivateChatHistoryResponse, [this](const network::Packet<network::MyPacketType>& packet) {
    HandlePrivateChatHistoryResponse(packet);
  });

  event_queue_.Subscribe(EventType::PrivateChatMessage, [this](const network::Packet<network::MyPacketType>& packet) {
    HandlePrivateChatMessage(packet);
  });

  network_server_.SendTcp(std::move(*network::CreateGetUserListPacket<network::MyPacketType>(0, 50)));
}

void ChatOverlay::InitializeUI() {
  input_box_ = std::make_unique<TextBox>(
      400, 640, 280, 40,
      std::make_unique<Text>(0, 0, "", font_, SDL_Color{255, 255, 255, 255}, renderer_.GetSDLRenderer()));

  auto send_text = std::make_unique<Text>(
      0, 0, "Send", font_, SDL_Color{255, 255, 255, 255}, renderer_.GetSDLRenderer());
  send_button_ = std::make_unique<TextButton>(
      700, 640, 60, 40, std::move(send_text));

  send_button_->SetRenderStrategy(ButtonState::Normal, [](SDL_Renderer* renderer, const Button& button) {
    SDL_SetRenderDrawColor(renderer, 30, 144, 255, 255);
    SDL_RenderFillRect(renderer, &button.GetBounds());
    SDL_SetRenderDrawColor(renderer, 25, 110, 220, 255);
    SDL_RenderDrawRect(renderer, &button.GetBounds());
  });

  send_button_->SetRenderStrategy(ButtonState::Hover, [](SDL_Renderer* renderer, const Button& button) {
    SDL_SetRenderDrawColor(renderer, 100, 149, 237, 255);
    SDL_RenderFillRect(renderer, &button.GetBounds());
    SDL_SetRenderDrawColor(renderer, 85, 130, 210, 255);
    SDL_RenderDrawRect(renderer, &button.GetBounds());
  });

  send_button_->SetRenderStrategy(ButtonState::Pressed, [](SDL_Renderer* renderer, const Button& button) {
    SDL_SetRenderDrawColor(renderer, 25, 110, 210, 255);
    SDL_RenderFillRect(renderer, &button.GetBounds());
    SDL_SetRenderDrawColor(renderer, 20, 90, 190, 255);
    SDL_RenderDrawRect(renderer, &button.GetBounds());
  });

  send_button_->SetOnClick([this]() { SendMessage(); });
}

void ChatOverlay::Render(SDL_Renderer* renderer) {
  for (const auto& button : user_buttons_) {
    button->Render(renderer);
  }

  if (selected_user_id_) {
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    constexpr SDL_Rect chat_area = {380, 20, 400, 680};
    SDL_RenderFillRect(renderer, &chat_area);

    int y_offset = 30;
    for (const auto& message : current_chat_) {
      const std::string message_text =
          (message.sender_id == *selected_user_id_ ? "[User]: " : "[You]: ") +
          message.content;

      const auto message_render = std::make_unique<Text>(
          400, y_offset, message_text, font_, SDL_Color{255, 255, 255, 255}, renderer);
      message_render->Render(renderer);

      y_offset += 20;
      if (y_offset > chat_area.h - 50) {
        break;
      }
    }

    if (input_box_) {
      input_box_->Render(renderer);
    }

    if (send_button_) {
      send_button_->Render(renderer);
    }
  }
}

void ChatOverlay::HandleInput(const SDL_Event& event) {
  for (const auto& button : user_buttons_) {
    button->HandleInput(event);
  }

  if (selected_user_id_) {
    if (input_box_) {
      input_box_->HandleInput(event);
    }

    if (send_button_) {
      send_button_->HandleInput(event);
    }

    if (event.type == SDL_MOUSEBUTTONDOWN) {
      const int x = event.button.x;
      const int y = event.button.y;
      if (constexpr SDL_Rect chat_area = {380, 20, 400, 680};
          !(x >= chat_area.x && x <= chat_area.x + chat_area.w &&
            y >= chat_area.y && y <= chat_area.y + chat_area.h)) {
        CloseChatArea();
            }
    }
  }
}

void ChatOverlay::HandleGetUserListResponse(const network::Packet<network::MyPacketType>& packet) {
  if (packet.body.size() < sizeof(int)) return;

  const int status_code = *reinterpret_cast<const int*>(packet.body.data());
  if (status_code != 200) return;

  constexpr size_t users_offset = sizeof(int);
  const size_t users_data_size = packet.body.size() - users_offset;
  if (users_data_size % sizeof(network::packets::GetUserListResponsePacket::UserInfo) != 0) return;

  const size_t user_count = users_data_size / sizeof(network::packets::GetUserListResponsePacket::UserInfo);
  const auto* user_data = reinterpret_cast<
      const network::packets::GetUserListResponsePacket::UserInfo*>(
      packet.body.data() + users_offset);

  const std::vector users(user_data, user_data + user_count);

  user_buttons_.clear();
  user_to_button_map_.clear();

  int y_offset = 20;
  for (const auto& [user_id, username, is_online] : users) {
    auto user_text = std::make_unique<Text>(
        0, 0,
        "[" + std::string(username) + "] is " + (is_online ? "Online" : "Offline"),
        font_, SDL_Color{255, 255, 255, 255}, renderer_.GetSDLRenderer());
    auto button = std::make_unique<TextButton>(20, y_offset, 300, 30, std::move(user_text));
    user_to_button_map_[button.get()] = user_id;

    button->SetRenderStrategy(ButtonState::Normal, [](SDL_Renderer* renderer, const Button& button) {
      SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
      SDL_RenderFillRect(renderer, &button.GetBounds());
      SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
      SDL_RenderDrawRect(renderer, &button.GetBounds());
    });

    button->SetRenderStrategy(ButtonState::Hover, [](SDL_Renderer* renderer, const Button& button) {
      SDL_SetRenderDrawColor(renderer, 192, 192, 192, 255);
      SDL_RenderFillRect(renderer, &button.GetBounds());
      SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
      SDL_RenderDrawRect(renderer, &button.GetBounds());
    });

    button->SetRenderStrategy(ButtonState::Pressed, [](SDL_Renderer* renderer, const Button& button) {
      SDL_SetRenderDrawColor(renderer, 0, 128, 255, 255);
      SDL_RenderFillRect(renderer, &button.GetBounds());
      SDL_SetRenderDrawColor(renderer, 0, 102, 204, 255);
      SDL_RenderDrawRect(renderer, &button.GetBounds());
    });

    button->SetOnClick([this, user_id = user_id]() {
      current_chat_.clear();
      SelectUser(user_id);
    });

    user_buttons_.push_back(std::move(button));
    y_offset += 40;
  }
}

void ChatOverlay::HandlePrivateChatHistoryResponse(const network::Packet<network::MyPacketType>& packet) {
  if (!selected_user_id_) {
    std::cerr << "[ChatOverlay][ERROR] No user selected when processing chat history." << std::endl;
    return;
  }

  if (packet.body.size() < sizeof(int)) {
    std::cerr << "[ChatOverlay][ERROR] Invalid PrivateChatHistoryResponsePacket size." << std::endl;
    return;
  }

  if (const int status_code = *reinterpret_cast<const int*>(packet.body.data());
      status_code != 200) {
    std::cerr << "[ChatOverlay][ERROR] Chat history request failed. Status code: "
              << status_code << std::endl;
    return;
  }

  constexpr size_t messages_offset = sizeof(int);
  const size_t messages_data_size = packet.body.size() - messages_offset;

  if (messages_data_size % sizeof(network::packets::PrivateChatHistoryResponsePacket::MessageInfo) != 0) {
    std::cerr << "[ChatOverlay][ERROR] Invalid message data size in PrivateChatHistoryResponsePacket." << std::endl;
    return;
  }

  const size_t message_count = messages_data_size / sizeof(network::packets::PrivateChatHistoryResponsePacket::MessageInfo);
  const auto* messages = reinterpret_cast<const network::packets::PrivateChatHistoryResponsePacket::MessageInfo*>(
      packet.body.data() + messages_offset);

  current_chat_.clear();

  for (size_t i = 0; i < message_count; ++i) {
    ChatMessage message = {
      messages[i].sender_id,
      messages[i].message,
      messages[i].message_id,
      messages[i].timestamp
    };
    std::cout << "[ChatOverlay] Loaded message: " << message.content << std::endl;
    current_chat_.push_back(std::move(message));
  }

  std::cout << "[ChatOverlay] Loaded " << message_count << " messages for user " << *selected_user_id_ << "." << std::endl;
}

void ChatOverlay::HandlePrivateChatMessage(const network::Packet<network::MyPacketType>& packet) {
  if (packet.body.size() != sizeof(network::packets::PrivateChatMessagePacket)) {
    std::cerr << "[ChatOverlay][ERROR] Invalid PrivateChatMessagePacket size." << std::endl;
    return;
  }

  const auto* message_packet =
      reinterpret_cast<const network::packets::PrivateChatMessagePacket*>(packet.body.data());

  if (selected_user_id_ &&
      (*selected_user_id_ == message_packet->sender_id || *selected_user_id_ == message_packet->recipient_id)) {
    ChatMessage message = {
      message_packet->sender_id,
      message_packet->message,
      message_packet->message_id,
      message_packet->timestamp
    };

    current_chat_.push_back(std::move(message));

    std::cout << "[ChatOverlay] New message received: " << message.content << std::endl;
      } else {
        std::cout << "[ChatOverlay] Message ignored: Not part of the selected conversation." << std::endl;
      }
}

void ChatOverlay::SelectUser(std::uint32_t user_id) {
  selected_user_id_ = user_id;

  if (last_selected_button_) {
    last_selected_button_->SetRenderStrategy(ButtonState::Normal, [](SDL_Renderer* renderer, const Button& button) {
      SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
      SDL_RenderFillRect(renderer, &button.GetBounds());
      SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
      SDL_RenderDrawRect(renderer, &button.GetBounds());
    });
  }

  for (const auto& button : user_buttons_) {
    if (user_to_button_map_[button.get()] == user_id) {
      button->SetRenderStrategy(ButtonState::Normal, [](SDL_Renderer* renderer, const Button& button) {
        SDL_SetRenderDrawColor(renderer, 100, 149, 237, 255);
        SDL_RenderFillRect(renderer, &button.GetBounds());
        SDL_SetRenderDrawColor(renderer, 85, 130, 210, 255);
        SDL_RenderDrawRect(renderer, &button.GetBounds());
      });

      last_selected_button_ = button.get();
      break;
    }
  }

  input_box_->SetVisible(true);
  send_button_->SetVisible(true);

  auto packet = network::CreatePrivateChatHistoryPacket<network::MyPacketType>(user_id);
  network_server_.SendTcp(std::move(*packet));
}


void ChatOverlay::SendMessage() const {
  const std::string message_content = input_box_->GetContent();
  if (message_content.empty()) return;

  auto packet = network::CreatePrivateMessagePacket<network::MyPacketType>(*selected_user_id_, message_content);
  if (!packet) {
    std::cerr << "[ChatOverlay][ERROR] Failed to create private message packet." << std::endl;
    return;
  }

  network_server_.SendTcp(std::move(*packet));
  input_box_->SetContent("");
}

void ChatOverlay::CloseChatArea() {
  selected_user_id_.reset();

  current_chat_.clear();

  if (last_selected_button_) {
    last_selected_button_->SetRenderStrategy(ButtonState::Normal, [](SDL_Renderer* renderer, const Button& button) {
      SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
      SDL_RenderFillRect(renderer, &button.GetBounds());
      SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
      SDL_RenderDrawRect(renderer, &button.GetBounds());
    });
    last_selected_button_ = nullptr;
  }
}

}  // namespace rtype
