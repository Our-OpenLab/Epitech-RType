#include "scenes/main_menu_scene.hpp"
#include "core/resource_manager.hpp"
#include "core/service_locator.hpp"
//#include "scenes/game_scene.hpp"
//#include "scenes/settings_scene.hpp"

namespace rtype {

MainMenuScene::MainMenuScene()
    : renderer_(ServiceLocator::Get<Renderer>()),
      scene_manager_(ServiceLocator::Get<SceneManager>()){
  if (ServiceLocator::Has<ChatOverlay>()) {
    chat_overlay_ = ServiceLocator::Get<std::shared_ptr<ChatOverlay>>();
  } else {
    chat_overlay_ = std::make_shared<ChatOverlay>();
    ServiceLocator::Provide(chat_overlay_);
  }

  InitializeUI();
}

void MainMenuScene::Enter() {
  std::cout << "[MainMenuScene] Enter()" << std::endl;
  SDL_StartTextInput();
}

void MainMenuScene::Exit() {
  std::cout << "[MainMenuScene] Exit()" << std::endl;
  SDL_StopTextInput();
}

void MainMenuScene::Update(float /*delta_time*/) {
  // Logic update (if necessary)
}

void MainMenuScene::Render() {
  if (title_text_) title_text_->Render(renderer_.GetSDLRenderer());
  if (play_button_) play_button_->Render(renderer_.GetSDLRenderer());
  if (settings_button_) settings_button_->Render(renderer_.GetSDLRenderer());
  if (exit_button_) exit_button_->Render(renderer_.GetSDLRenderer());

  if (chat_overlay_) chat_overlay_->Render(renderer_.GetSDLRenderer());
}

void MainMenuScene::HandleInput(const SDL_Event& event) {
  if (play_button_) play_button_->HandleInput(event);
  if (settings_button_) settings_button_->HandleInput(event);
  if (exit_button_) exit_button_->HandleInput(event);

  if (chat_overlay_) chat_overlay_->HandleInput(event);
}

void MainMenuScene::InitializeUI() {
  const std::string font_path = "assets/fonts/Roboto-Regular.ttf";
  constexpr int font_size = 36;
  SDL_Color white_color{255, 255, 255, 255};

  // Load font
  if (!GlobalResourceManager::Instance().LoadFont(font_path, font_path, font_size)) {
    throw std::runtime_error("[MainMenuScene] Failed to load font: Roboto-Regular.");
  }
  font_ = GlobalResourceManager::Instance().GetFont(font_path);
  if (!font_) {
    throw std::runtime_error("[MainMenuScene] Font was loaded but could not be retrieved.");
  }

  // Title
  title_text_ = std::make_unique<Text>(
      100, 50, "Main Menu", font_, white_color, renderer_.GetSDLRenderer());

  // Play Button
  auto play_text = std::make_unique<Text>(
      0, 0, "Play", font_, white_color, renderer_.GetSDLRenderer());
  play_button_ = std::make_unique<TextButton>(
      100, 150, 200, 50, std::move(play_text));
  play_button_->SetRenderStrategy(ButtonState::Normal, [](SDL_Renderer* renderer, const Button& button) {
    SDL_SetRenderDrawColor(renderer, 0, 128, 0, 255);  // Green
    SDL_RenderFillRect(renderer, &button.GetBounds());
  });
  play_button_->SetRenderStrategy(ButtonState::Hover, [](SDL_Renderer* renderer, const Button& button) {
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);  // Bright Green
    SDL_RenderFillRect(renderer, &button.GetBounds());
  });
  play_button_->SetOnClick([this]() { OnPlayButtonClicked(); });

  // Settings Button
  auto settings_text = std::make_unique<Text>(
      0, 0, "Settings", font_, white_color, renderer_.GetSDLRenderer());
  settings_button_ = std::make_unique<TextButton>(
      100, 220, 200, 50, std::move(settings_text));
  settings_button_->SetRenderStrategy(ButtonState::Normal, [](SDL_Renderer* renderer, const Button& button) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 128, 255);  // Blue
    SDL_RenderFillRect(renderer, &button.GetBounds());
  });
  settings_button_->SetRenderStrategy(ButtonState::Hover, [](SDL_Renderer* renderer, const Button& button) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);  // Bright Blue
    SDL_RenderFillRect(renderer, &button.GetBounds());
  });
  settings_button_->SetOnClick([this]() { OnSettingsButtonClicked(); });

  // Exit Button
  auto exit_text = std::make_unique<Text>(
      0, 0, "Exit", font_, white_color, renderer_.GetSDLRenderer());
  exit_button_ = std::make_unique<TextButton>(
      100, 290, 200, 50, std::move(exit_text));
  exit_button_->SetRenderStrategy(ButtonState::Normal, [](SDL_Renderer* renderer, const Button& button) {
    SDL_SetRenderDrawColor(renderer, 128, 0, 0, 255);  // Red
    SDL_RenderFillRect(renderer, &button.GetBounds());
  });
  exit_button_->SetRenderStrategy(ButtonState::Hover, [](SDL_Renderer* renderer, const Button& button) {
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // Bright Red
    SDL_RenderFillRect(renderer, &button.GetBounds());
  });
  exit_button_->SetOnClick([this]() { OnExitButtonClicked(); });
}

void MainMenuScene::OnPlayButtonClicked() {
  std::cout << "[MainMenuScene] Play button clicked. Transitioning to GameScene." << std::endl;
 // scene_manager_.ReplaceScene(std::make_unique<GameScene>());
}

void MainMenuScene::OnSettingsButtonClicked() {
  std::cout << "[MainMenuScene] Settings button clicked. Transitioning to SettingsScene." << std::endl;
 // scene_manager_.ReplaceScene(std::make_unique<SettingsScene>());
}

void MainMenuScene::OnExitButtonClicked() {
  std::cout << "[MainMenuScene] Exit button clicked. Exiting application." << std::endl;
  SDL_Quit();
  exit(0);
}

}  // namespace rtype
