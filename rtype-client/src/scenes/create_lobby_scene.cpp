#include "scenes/create_lobby_scene.hpp"

#include <iostream>
#include <string>

#include "core/resource_manager.hpp"
#include "scenes/lobby_scene.hpp"
#include "scenes/main_menu_scene.hpp"

namespace rtype {

CreateLobbyScene::CreateLobbyScene()
    : renderer_(ServiceLocator::Get<Renderer>("renderer")),
      scene_manager_(ServiceLocator::Get<SceneManager>("scene_manager")),
      event_queue_(ServiceLocator::Get<EventQueue<network::Packet<network::MyPacketType>>>("event_queue")),
      network_server_(ServiceLocator::Get<network::NetworkClient<network::MyPacketType>>("network_server")) {
  InitializeUI();
}

void CreateLobbyScene::InitializeUI() {
  const std::string font_path = "assets/fonts/Roboto-Regular.ttf";
  constexpr int font_size = 24;
  SDL_Color white_color{255, 255, 255, 255};

  if (!GlobalResourceManager::Instance().LoadFont(font_path, font_path, font_size)) {
    throw std::runtime_error("[CreateLobbyScene] Failed to load font: Roboto-Regular.");
  }

  font_ = GlobalResourceManager::Instance().GetFont(font_path);
  if (!font_) {
    throw std::runtime_error("[CreateLobbyScene] Font was loaded but could not be retrieved.");
  }

  // Title
  title_text_ = std::make_unique<Text>(
      100, 40, "Create Lobby", font_, white_color, renderer_.GetSDLRenderer());

  // Lobby Name
  name_label_ = std::make_unique<Text>(
      100, 100, "Lobby Name:", font_, white_color, renderer_.GetSDLRenderer());
  name_box_ = std::make_unique<TextBox>(
      250, 95, 400, 40,
      std::make_unique<Text>(0, 0, "", font_, white_color, renderer_.GetSDLRenderer()), 32);

  // Password
  password_label_ = std::make_unique<Text>(
      100, 160, "Password (Optional):", font_, white_color, renderer_.GetSDLRenderer());
  password_box_ = std::make_unique<TextBox>(
      250, 155, 400, 40,
      std::make_unique<Text>(0, 0, "", font_, white_color, renderer_.GetSDLRenderer()), 32);

  // Create Button
  auto create_text = std::make_unique<Text>(
      0, 0, "Create", font_, white_color, renderer_.GetSDLRenderer());
  create_button_ = std::make_unique<TextButton>(
      100, 220, 150, 50, std::move(create_text));
  create_button_->SetRenderStrategy(ButtonState::Normal, [](SDL_Renderer* renderer_, const Button& button) {
    SDL_SetRenderDrawColor(renderer_, 0, 128, 0, 255);  // Green
    SDL_RenderFillRect(renderer_, &button.GetBounds());
  });
  create_button_->SetRenderStrategy(ButtonState::Hover, [](SDL_Renderer* renderer_, const Button& button) {
    SDL_SetRenderDrawColor(renderer_, 0, 255, 0, 255);  // Bright Green
    SDL_RenderFillRect(renderer_, &button.GetBounds());
  });
  create_button_->SetOnClick([this]() { OnCreateButtonClicked(); });

  // Cancel Button
  auto cancel_text = std::make_unique<Text>(
      0, 0, "Cancel", font_, white_color, renderer_.GetSDLRenderer());
  cancel_button_ = std::make_unique<TextButton>(
      300, 220, 150, 50, std::move(cancel_text));
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
      100, 300, "", font_, SDL_Color{255, 0, 0, 255}, renderer_.GetSDLRenderer());
}

void CreateLobbyScene::OnCreateButtonClicked() {
  const std::string name = name_box_->GetContent();
  const std::string password = password_box_->GetContent();

  if (name.empty()) {
    status_text_->SetContent("Error: Lobby name cannot be empty.");
    return;
  }

  auto create_lobby_packet =
      network::CreateCreateLobbyPacket<network::MyPacketType>(
      name,
      password.empty() ? std::nullopt : std::optional(password)
  );

  if (!create_lobby_packet) {
    status_text_->SetContent("Error: Failed to create lobby packet. Name or password too long.");
    return;
  }

  network_server_.SendTcp(std::move(*create_lobby_packet));
  status_text_->SetContent("Creating lobby...");
}


void CreateLobbyScene::OnCancelButtonClicked() {
  std::cout << "[CreateLobbyScene] Cancel button clicked. Returning to MainMenuScene." << std::endl;
  scene_manager_.ReplaceScene(std::make_unique<MainMenuScene>());
}

void CreateLobbyScene::Enter() {
  std::cout << "[CreateLobbyScene] Enter()" << std::endl;
  SDL_StartTextInput();

  // Subscribe to CreateLobbyResponse event
  event_queue_.Subscribe(EventType::CreateLobbyResponse, [this](const network::Packet<network::MyPacketType>& packet) {
    if (packet.body.size() != sizeof(network::packets::CreateLobbyResponsePacket)) {
      status_text_->SetContent("Error: Invalid server response.");
      return;
    }
    const auto* response = reinterpret_cast<const network::packets::CreateLobbyResponsePacket*>(packet.body.data());
    HandleCreateLobbyResponse(*response);
  });
}

void CreateLobbyScene::HandleCreateLobbyResponse(const network::packets::CreateLobbyResponsePacket& packet) {
  if (packet.status_code == 200) {
    status_text_->SetContent("Lobby created successfully!");
    status_text_->SetColor(SDL_Color{0, 255, 0, 255});
    std::cout << "[CreateLobbyScene] Lobby created successfully. Transitioning to LobbyScene." << std::endl;

    // Pass the lobby_id to LobbyScene
    scene_manager_.ReplaceScene(std::make_unique<LobbyScene>(packet.lobby_id));
  } else {
    status_text_->SetContent("Error: Failed to create lobby.");
    std::cout << "[CreateLobbyScene] Failed to create lobby. Status code: " << packet.status_code << std::endl;
  }
}

void CreateLobbyScene::Exit() {
  std::cout << "[CreateLobbyScene] Exit()" << std::endl;
  SDL_StopTextInput();

  // Clear handlers for the CreateLobbyResponse event
  event_queue_.ClearHandlers(EventType::CreateLobbyResponse);
}

void CreateLobbyScene::Update(float /*delta_time*/) {}

void CreateLobbyScene::Render() {
  if (title_text_) title_text_->Render(renderer_.GetSDLRenderer());
  if (name_label_) name_label_->Render(renderer_.GetSDLRenderer());
  if (name_box_) name_box_->Render(renderer_.GetSDLRenderer());
  if (password_label_) password_label_->Render(renderer_.GetSDLRenderer());
  if (password_box_) password_box_->Render(renderer_.GetSDLRenderer());
  if (create_button_) create_button_->Render(renderer_.GetSDLRenderer());
  if (cancel_button_) cancel_button_->Render(renderer_.GetSDLRenderer());
  if (status_text_) status_text_->Render(renderer_.GetSDLRenderer());
}

void CreateLobbyScene::HandleInput(const SDL_Event& event) {
  if (name_box_) name_box_->HandleInput(event);
  if (password_box_) password_box_->HandleInput(event);
  if (create_button_) create_button_->HandleInput(event);
  if (cancel_button_) cancel_button_->HandleInput(event);
}

}  // namespace rtype
