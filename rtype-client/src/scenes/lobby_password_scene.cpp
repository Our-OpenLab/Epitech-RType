#include "scenes/lobby_password_scene.hpp"

#include <iostream>
#include <string>
#include "core/resource_manager.hpp"
#include "scenes/main_menu_scene.hpp"
#include "scenes/lobby_scene.hpp"

namespace rtype {

LobbyPasswordScene::LobbyPasswordScene(const int lobby_id)
    : lobby_id_(lobby_id),
      renderer_(ServiceLocator::Get<Renderer>("renderer")),
      scene_manager_(ServiceLocator::Get<SceneManager>("scene_manager")),
      event_queue_(ServiceLocator::Get<EventQueue<network::Packet<network::MyPacketType>>>("event_queue")),
      network_server_(ServiceLocator::Get<network::NetworkClient<network::MyPacketType>>("network_server")) {
  InitializeUI();
}

void LobbyPasswordScene::InitializeUI() {
  const std::string font_path = "assets/fonts/Roboto-Regular.ttf";
  constexpr int font_size = 24;
  SDL_Color white_color{255, 255, 255, 255};

  if (!GlobalResourceManager::Instance().LoadFont(font_path, font_path, font_size)) {
    throw std::runtime_error("[LobbyPasswordScene] Failed to load font: Roboto-Regular.");
  }

  font_ = GlobalResourceManager::Instance().GetFont(font_path);
  if (!font_) {
    throw std::runtime_error("[LobbyPasswordScene] Font was loaded but could not be retrieved.");
  }

  // Title
  title_text_ = std::make_unique<Text>(
      100, 40, "Enter Lobby Password", font_, white_color, renderer_.GetSDLRenderer());

  // Password Input
  password_label_ = std::make_unique<Text>(
      100, 100, "Password:", font_, white_color, renderer_.GetSDLRenderer());
  password_box_ = std::make_unique<TextBox>(
      250, 95, 400, 40,
      std::make_unique<Text>(0, 0, "", font_, white_color, renderer_.GetSDLRenderer()), 32);

  // Submit Button
  auto submit_text = std::make_unique<Text>(
      0, 0, "Join", font_, white_color, renderer_.GetSDLRenderer());
  submit_button_ = std::make_unique<TextButton>(
      100, 160, 150, 50, std::move(submit_text));
  submit_button_->SetRenderStrategy(ButtonState::Normal, [](SDL_Renderer* renderer_, const Button& button) {
    SDL_SetRenderDrawColor(renderer_, 0, 128, 0, 255);  // Green
    SDL_RenderFillRect(renderer_, &button.GetBounds());
  });
  submit_button_->SetRenderStrategy(ButtonState::Hover, [](SDL_Renderer* renderer_, const Button& button) {
    SDL_SetRenderDrawColor(renderer_, 0, 255, 0, 255);  // Bright Green
    SDL_RenderFillRect(renderer_, &button.GetBounds());
  });
  submit_button_->SetOnClick([this]() { OnSubmitButtonClicked(); });

  // Cancel Button
  auto cancel_text = std::make_unique<Text>(
      0, 0, "Cancel", font_, white_color, renderer_.GetSDLRenderer());
  cancel_button_ = std::make_unique<TextButton>(
      300, 160, 150, 50, std::move(cancel_text));
  cancel_button_->SetRenderStrategy(ButtonState::Normal, [](SDL_Renderer* renderer_, const Button& button) {
    SDL_SetRenderDrawColor(renderer_, 128, 0, 0, 255);  // Red
    SDL_RenderFillRect(renderer_, &button.GetBounds());
  });
  cancel_button_->SetRenderStrategy(ButtonState::Hover, [](SDL_Renderer* renderer_, const Button& button) {
    SDL_SetRenderDrawColor(renderer_, 255, 0, 0, 255);  // Bright Red
    SDL_RenderFillRect(renderer_, &button.GetBounds());
  });
  cancel_button_->SetOnClick([this]() { OnCancelButtonClicked(); });

  // Status Text
  status_text_ = std::make_unique<Text>(
      100, 230, "", font_, SDL_Color{255, 0, 0, 255}, renderer_.GetSDLRenderer());
}

void LobbyPasswordScene::OnSubmitButtonClicked() {
  const std::string password = password_box_->GetContent();

  auto join_lobby_packet =
      network::CreateJoinLobbyPacket<network::MyPacketType>(lobby_id_, password);

  if (!join_lobby_packet) {
    status_text_->SetContent("Error: Failed to create join lobby packet.");
    return;
  }

  network_server_.SendTcp(std::move(*join_lobby_packet));
  status_text_->SetContent("Joining lobby...");
}

void LobbyPasswordScene::OnCancelButtonClicked() {
  std::cout << "[LobbyPasswordScene] Cancel button clicked. Returning to MainMenuScene." << std::endl;
  scene_manager_.ReplaceScene(std::make_unique<MainMenuScene>());
}

void LobbyPasswordScene::Enter() {
  std::cout << "[LobbyPasswordScene] Enter()" << std::endl;
  SDL_StartTextInput();

  event_queue_.Subscribe(EventType::JoinLobbyResponse, [this](const network::Packet<network::MyPacketType>& packet) {
    if (packet.body.size() != sizeof(network::packets::JoinLobbyResponsePacket)) {
      status_text_->SetContent("Error: Invalid server response.");
      return;
    }
    const auto* response = reinterpret_cast<const network::packets::JoinLobbyResponsePacket*>(packet.body.data());
    HandleJoinLobbyResponse(*response);
  });
}

void LobbyPasswordScene::HandleJoinLobbyResponse(const network::packets::JoinLobbyResponsePacket& packet) {
  if (packet.status_code == 200) {
    status_text_->SetContent("Joined lobby successfully!");
    status_text_->SetColor(SDL_Color{0, 255, 0, 255});
    std::cout << "[LobbyPasswordScene] Successfully joined lobby. Transitioning to LobbyScene." << std::endl;

    // Transition to LobbyScene
    scene_manager_.ReplaceScene(std::make_unique<LobbyScene>(lobby_id_));
  } else {
    status_text_->SetContent("Error: Incorrect password.");
    std::cout << "[LobbyPasswordScene] Failed to join lobby. Status code: " << packet.status_code << std::endl;
  }
}

void LobbyPasswordScene::Exit() {
  std::cout << "[LobbyPasswordScene] Exit()" << std::endl;
  SDL_StopTextInput();

  event_queue_.ClearHandlers(EventType::JoinLobbyResponse);
}

void LobbyPasswordScene::Update(float /*delta_time*/) {}

void LobbyPasswordScene::Render() {
  if (title_text_) title_text_->Render(renderer_.GetSDLRenderer());
  if (password_label_) password_label_->Render(renderer_.GetSDLRenderer());
  if (password_box_) password_box_->Render(renderer_.GetSDLRenderer());
  if (submit_button_) submit_button_->Render(renderer_.GetSDLRenderer());
  if (cancel_button_) cancel_button_->Render(renderer_.GetSDLRenderer());
  if (status_text_) status_text_->Render(renderer_.GetSDLRenderer());
}

void LobbyPasswordScene::HandleInput(const SDL_Event& event) {
  if (password_box_) password_box_->HandleInput(event);
  if (cancel_button_) cancel_button_->HandleInput(event);
  if (submit_button_) submit_button_->HandleInput(event);
}

}  // namespace rtype
