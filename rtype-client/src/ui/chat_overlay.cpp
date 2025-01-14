#include "ui/chat_overlay.hpp"

#include "core/packet_factory.hpp"
#include "core/resource_manager.hpp"
#include "core/service_locator.hpp"

namespace rtype {

ChatOverlay::ChatOverlay()
    : font_(GlobalResourceManager::Instance().GetFont("assets/fonts/Roboto-Regular.ttf")),
      renderer_(ServiceLocator::Get<Renderer>()),
      network_server_(ServiceLocator::Get<network::NetworkClient<network::MyPacketType>>()),
      event_queue_(ServiceLocator::Get<EventQueue<network::Packet<network::MyPacketType>>>()) {
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

  network_server_.SendTcp(std::move(*network::CreateGetUserListPacket<network::MyPacketType>(0, 50)));
}

void ChatOverlay::InitializeUI() {
  // No static UI initialization required
}

void ChatOverlay::Render(SDL_Renderer* renderer) {
  // Render user buttons
  for (const auto& button : user_buttons_) {
    button->Render(renderer);
  }

  // Render chat area if a user is selected
  if (selected_user_id_) {
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_Rect chat_area = {420, 0, 300, 600};
    SDL_RenderFillRect(renderer, &chat_area);

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

    // Close the chat area if clicked outside
    if (event.type == SDL_MOUSEBUTTONDOWN) {
      int x = event.button.x, y = event.button.y;
      SDL_Rect chat_area = {420, 0, 300, 600};
      if (!(x >= chat_area.x && x <= chat_area.x + chat_area.w &&
            y >= chat_area.y && y <= chat_area.y + chat_area.h)) {
        CloseChatArea();
            }
    }
  }
}

void ChatOverlay::HandleGetUserListResponse(
    const network::Packet<network::MyPacketType>& packet) {
  if (packet.body.size() < sizeof(int)) {
    std::cerr << "[ChatOverlay][ERROR] Invalid GetUserListResponsePacket size."
              << std::endl;
    return;
  }

  if (const int status_code = *reinterpret_cast<const int*>(packet.body.data());
      status_code != 200) {
    return;
  }

  constexpr size_t users_offset = sizeof(int);
  const size_t users_data_size = packet.body.size() - users_offset;

  if (users_data_size %
          sizeof(network::packets::GetUserListResponsePacket::UserInfo) !=
      0) {
    std::cerr << "[ChatOverlay][ERROR] Invalid user data size in "
                 "GetUserListResponsePacket."
              << std::endl;
    return;
  }

  const size_t user_count =
      users_data_size /
      sizeof(network::packets::GetUserListResponsePacket::UserInfo);
  const auto* user_data = reinterpret_cast<
      const network::packets::GetUserListResponsePacket::UserInfo*>(
      packet.body.data() + users_offset);
  const std::vector users(user_data, user_data + user_count);

  if (users.empty()) {
    return;
  }

  user_buttons_.clear();
  user_to_button_map_.clear();

  int y_offset = 20;
  for (const auto& user : users) {
    std::string user_status = user.is_online ? "Online" : "Offline";
    std::string button_text =
        "[" + std::string(user.username) + "] is " + user_status;

    auto user_text = std::make_unique<Text>(0, 0, button_text, font_,
                                            SDL_Color{255, 255, 255, 255},
                                            renderer_.GetSDLRenderer());
    auto button = std::make_unique<TextButton>(20, y_offset, 300, 30,
                                               std::move(user_text));

    const auto user_id = user.user_id;
    user_to_button_map_[button.get()] = user_id;

    button->SetOnClick([this, user_id]() { SelectUser(user_id); });

    user_buttons_.push_back(std::move(button));
    y_offset += 40;
  }

  std::cout << "[ChatOverlay] Displayed user list." << std::endl;
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

/*
void ChatOverlay::SelectUser(std::uint32_t user_id) {
  selected_user_id_ = user_id;

  input_box_ = std::make_unique<TextBox>(
      430, 560, 280, 30,
      std::make_unique<Text>(0, 0, "", font_, SDL_Color{255, 255, 255, 255},
                             renderer_.GetSDLRenderer()));

  for (const auto& button : user_buttons_) {
    if (user_to_button_map_[button.get()] == user_id) {
      button->SetRenderStrategy(ButtonState::Normal, [](SDL_Renderer* renderer, const Button& button) {
        SDL_SetRenderDrawColor(renderer, 0, 128, 255, 255);  // Highlight color
        SDL_RenderFillRect(renderer, &button.GetBounds());
      });
    } else {
      button->SetRenderStrategy(ButtonState::Normal, [](SDL_Renderer* renderer, const Button& button) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Default color
        SDL_RenderFillRect(renderer, &button.GetBounds());
      });
    }
  }

  auto packet = network::CreatePrivateChatHistoryPacket<network::MyPacketType>(user_id);
  network_server_.SendTcp(std::move(*packet));
}
*/

void ChatOverlay::SelectUser(std::uint32_t user_id) {
  selected_user_id_ = user_id;

  // Création de la boîte de texte pour la saisie utilisateur
  input_box_ = std::make_unique<TextBox>(
      430, 560, 220, 30,
      std::make_unique<Text>(0, 0, "", font_, SDL_Color{255, 255, 255, 255}, renderer_.GetSDLRenderer()));

  // Création du texte du bouton "Send"
  auto send_text = std::make_unique<Text>(
      0, 0, "Send", font_, SDL_Color{255, 255, 255, 255}, renderer_.GetSDLRenderer());

  // Création du bouton "Send"
  send_button_ = std::make_unique<TextButton>(
      660, 560, 60, 30, std::move(send_text));

  // Définir les stratégies de rendu pour le bouton
  send_button_->SetRenderStrategy(ButtonState::Normal, [](SDL_Renderer* renderer, const Button& button) {
    // Couleur normale : Bleu clair avec coins arrondis (effet moderne)
    SDL_SetRenderDrawColor(renderer, 30, 144, 255, 255);  // DodgerBlue
    SDL_RenderFillRect(renderer, &button.GetBounds());

    SDL_SetRenderDrawColor(renderer, 25, 110, 220, 255);  // Bordure plus sombre
    SDL_RenderDrawRect(renderer, &button.GetBounds());
  });

  send_button_->SetRenderStrategy(ButtonState::Hover, [](SDL_Renderer* renderer, const Button& button) {
    // Couleur survolée : Bleu légèrement plus clair
    SDL_SetRenderDrawColor(renderer, 100, 149, 237, 255);  // CornflowerBlue
    SDL_RenderFillRect(renderer, &button.GetBounds());

    SDL_SetRenderDrawColor(renderer, 85, 130, 210, 255);  // Bordure ajustée
    SDL_RenderDrawRect(renderer, &button.GetBounds());
  });

  send_button_->SetRenderStrategy(ButtonState::Pressed, [](SDL_Renderer* renderer, const Button& button) {
    // Couleur cliquée : Accentuation du bleu avec un léger foncé
    SDL_SetRenderDrawColor(renderer, 25, 110, 210, 255);  // Bleu foncé
    SDL_RenderFillRect(renderer, &button.GetBounds());

    SDL_SetRenderDrawColor(renderer, 20, 90, 190, 255);  // Bordure encore plus sombre
    SDL_RenderDrawRect(renderer, &button.GetBounds());
  });

  // Définir l'action du clic sur le bouton
  send_button_->SetOnClick([this]() { SendMessage(); });

  // Envoyer un paquet pour demander l'historique des messages
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
  input_box_ = nullptr;
  send_button_ = nullptr;
}

}  // namespace rtype
