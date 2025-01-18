#include "scenes/game_scene.hpp"
#include <iostream>
#include "core/resource_manager.hpp"

namespace rtype {

GameScene::GameScene(const std::string& ip_address, const std::vector<int>& ports)
    : ip_address_(ip_address),
      ports_(ports),
      is_connected_(false),
      renderer_(ServiceLocator::Get<Renderer>()) {
  InitializeUI();
}

void GameScene::InitializeUI() {
  const std::string font_path = "assets/fonts/Roboto-Regular.ttf";
  constexpr int font_size = 24;
  SDL_Color white_color{255, 255, 255, 255};

  if (!GlobalResourceManager::Instance().LoadFont(font_path, font_path, font_size)) {
    throw std::runtime_error("[GameScene] Failed to load font: Roboto-Regular.");
  }

  font_ = GlobalResourceManager::Instance().GetFont(font_path);
  if (!font_) {
    throw std::runtime_error("[GameScene] Font was loaded but could not be retrieved.");
  }

  connection_status_text_ = std::make_unique<Text>(
      100, 100, "Connecting to game server...", font_, white_color, renderer_.GetSDLRenderer());
}

void GameScene::Enter() {
  std::cout << "[GameScene] Enter()" << std::endl;
  ConnectToGameServer();
}

void GameScene::Exit() {
  std::cout << "[GameScene] Exit()" << std::endl;
}

void GameScene::Update(float /*delta_time*/) {
  // Update game logic here if needed
}

void GameScene::Render() {
  if (connection_status_text_) connection_status_text_->Render(renderer_.GetSDLRenderer());
}

void GameScene::HandleInput(const SDL_Event& /*event*/) {
  // Handle user input here if needed
}

void GameScene::ConnectToGameServer() {
  std::cout << "[GameScene] Attempting to connect to " << ip_address_ << " on ports: ";
  for (const auto& port : ports_) {
    std::cout << port << " ";
  }
  std::cout << std::endl;

  // Simulate a connection attempt (replace with actual network logic)
  is_connected_ = true;

  if (is_connected_) {
    connection_status_text_->SetContent("Connected to game server!");
    connection_status_text_->SetColor(SDL_Color{0, 255, 0, 255});
    std::cout << "[GameScene] Successfully connected to the game server." << std::endl;
  } else {
    connection_status_text_->SetContent("Failed to connect to game server.");
    connection_status_text_->SetColor(SDL_Color{255, 0, 0, 255});
    std::cerr << "[GameScene][ERROR] Connection to game server failed." << std::endl;
  }
}

}  // namespace rtype
