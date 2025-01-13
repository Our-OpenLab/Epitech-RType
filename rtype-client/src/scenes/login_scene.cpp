#include "scenes/login_scene.hpp"

#include <network/network_client.hpp>

#include "scenes/main_menu_scene.hpp"
#include "scenes/register_scene.hpp"

namespace rtype {

LoginScene::LoginScene()
    : is_waiting_(false),
      renderer_(ServiceLocator::Get<Renderer>()),
      scene_manager_(ServiceLocator::Get<SceneManager>()),
      event_queue_(ServiceLocator::Get<EventQueue<network::Packet<network::MyPacketType>>>()),
      network_server_(ServiceLocator::Get<
                      network::NetworkClient<network::MyPacketType>>()) {
  InitializeResources();
  CreateUIElements();
}

void LoginScene::InitializeResources() {
  const std::string font_path = "assets/fonts/Roboto-Regular.ttf";
  constexpr int font_size = 24;

  if (!GlobalResourceManager::Instance().LoadFont(font_path, font_path, font_size)) {
    throw std::runtime_error("[LoginScene] Failed to load font: Roboto-Regular.");
  }

  font_ = GlobalResourceManager::Instance().GetFont(font_path);
  if (!font_) {
    throw std::runtime_error("[LoginScene] Font was loaded but could not be retrieved.");
  }
}

void LoginScene::CreateUIElements() {
  SDL_Color white_color{255, 255, 255, 255};

  title_text_ = std::make_unique<Text>(
      100, 40, "Please Login:", font_, white_color, renderer_.GetSDLRenderer());

  username_label_ = std::make_unique<Text>(
      100, 100, "Username:", font_, white_color, renderer_.GetSDLRenderer());

  username_box_ = std::make_unique<TextBox>(
      230, 95, 400, 40,
      std::make_unique<Text>(
          0, 0, "", font_, white_color, renderer_.GetSDLRenderer()), 32);

  password_label_ = std::make_unique<Text>(
      100, 160, "Password:", font_, white_color, renderer_.GetSDLRenderer());

  password_box_ = std::make_unique<TextBox>(
      230, 155, 400, 40,
      std::make_unique<Text>(
          0, 0, "", font_, white_color, renderer_.GetSDLRenderer()), 32);

  auto login_text = std::make_unique<Text>(
      0, 0, "Login", font_, SDL_Color{255, 255, 255, 255}, renderer_.GetSDLRenderer());

  login_button_ = std::make_unique<TextButton>(
      100, 220, 150, 50, std::move(login_text));

  login_button_->SetRenderStrategy(ButtonState::Normal, [](SDL_Renderer* renderer_, const Button& button) {
    SDL_SetRenderDrawColor(renderer_, 0, 0, 255, 255);
    SDL_RenderFillRect(renderer_, &button.GetBounds());
  });

  login_button_->SetRenderStrategy(ButtonState::Hover, [](SDL_Renderer* renderer_, const Button& button) {
    SDL_SetRenderDrawColor(renderer_, 0, 128, 255, 255);
    SDL_RenderFillRect(renderer_, &button.GetBounds());
  });

  login_button_->SetOnClick([this]() { OnLoginButtonClicked(); });

  register_link_ = std::make_unique<TextLink>(
      100, 290, "Don't have an account? Register here.",
      font_, white_color, SDL_Color{128, 0, 255, 255}, renderer_.GetSDLRenderer());

  register_link_->SetOnClick([this]() {
    std::cout << "[LoginScene] Register link clicked. Transitioning to RegisterScene." << std::endl;
    scene_manager_.ReplaceScene(std::make_unique<RegisterScene>());
  });

  error_text_ = std::make_unique<Text>(
      100, 350, "", font_, SDL_Color{255, 0, 0, 255}, renderer_.GetSDLRenderer());
}

void LoginScene::OnLoginButtonClicked() {
  if (is_waiting_) return;

  const std::string username = username_box_->GetContent();
  const std::string password = password_box_->GetContent();

  if (username.empty() || password.empty()) {
    error_text_->SetContent("Error: Both fields must be filled.");
    return;
  }

  is_waiting_ = true;

  std::cout << "[LoginScene] Login attempt: username='" << username
            << "', password='" << password << "'" << std::endl;

  if (auto login_packet = network::CreateLoginPacket<network::MyPacketType>(
          username, password)) {
    network_server_.SendTcp(std::move(*login_packet));
  } else {
    error_text_->SetContent("Error: Failed to create login packet.");
    is_waiting_ = false;
  }
}

void LoginScene::HandleLoginResponse(const network::packets::LoginResponsePacket& packet) {
  if (packet.status_code == 200) {
    std::cout << "[LoginScene] Login successful. Transitioning to next scene." << std::endl;
    is_waiting_ = false;
    scene_manager_.ReplaceScene(std::make_unique<MainMenuScene>());
  } else {
    std::cout << "[LoginScene] Login failed. Status code: " << packet.status_code << std::endl;
    error_text_->SetContent("Login failed. Please try again.");
    is_waiting_ = false;
  }
}

void LoginScene::Enter() {
  std::cout << "[LoginScene] Enter()" << std::endl;
  SDL_StartTextInput();

  event_queue_.Subscribe(EventType::LoginResponse, [this](const network::Packet<network::MyPacketType>& packet) {
    if (packet.body.size() != sizeof(network::packets::LoginResponsePacket)) {
      error_text_->SetContent("Error: Invalid server response.");
      is_waiting_ = false;
      return;
    }
    const auto* response = reinterpret_cast<const network::packets::LoginResponsePacket*>(packet.body.data());
    HandleLoginResponse(*response);
  });
}

void LoginScene::Exit() {
  std::cout << "[LoginScene] Exit()" << std::endl;
  SDL_StopTextInput();
  event_queue_.ClearHandlers(EventType::LoginResponse);
}

void LoginScene::Update(float /*delta_time*/) {}

void LoginScene::Render() {
  if (title_text_) title_text_->Render(renderer_.GetSDLRenderer());
  if (username_label_) username_label_->Render(renderer_.GetSDLRenderer());
  if (username_box_) username_box_->Render(renderer_.GetSDLRenderer());
  if (password_label_) password_label_->Render(renderer_.GetSDLRenderer());
  if (password_box_) password_box_->Render(renderer_.GetSDLRenderer());
  if (login_button_) login_button_->Render(renderer_.GetSDLRenderer());
  if (register_link_) register_link_->Render(renderer_.GetSDLRenderer());
  if (error_text_) error_text_->Render(renderer_.GetSDLRenderer());
}

void LoginScene::HandleInput(const SDL_Event& e) {
  if (username_box_) username_box_->HandleInput(e);
  if (password_box_) password_box_->HandleInput(e);
  if (login_button_) login_button_->HandleInput(e);
  if (register_link_) register_link_->HandleInput(e);
}

}  // namespace rtype
