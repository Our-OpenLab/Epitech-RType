#include <network/network_client.hpp>

#include "scenes/register_scene.hpp"
#include "scenes/login_scene.hpp"

namespace rtype {

RegisterScene::RegisterScene()
    : is_waiting_(false),
      renderer_(ServiceLocator::Get<Renderer>("renderer")),
      scene_manager_(ServiceLocator::Get<SceneManager>("scene_manager")),
      event_queue_(ServiceLocator::Get<EventQueue<network::Packet<network::MyPacketType>>>("event_queue")),
      network_server_(ServiceLocator::Get<network::NetworkClient<network::MyPacketType>>("network_server")) {
  InitializeResources();
  CreateUIElements();
}

void RegisterScene::InitializeResources() {
  const std::string font_path = "assets/fonts/Roboto-Regular.ttf";
  constexpr int font_size = 24;

  if (!GlobalResourceManager::Instance().LoadFont(font_path, font_path, font_size)) {
    throw std::runtime_error("[RegisterScene] Failed to load font: Roboto-Regular.");
  }

  font_ = GlobalResourceManager::Instance().GetFont(font_path);
  if (!font_) {
    throw std::runtime_error("[RegisterScene] Font was loaded but could not be retrieved.");
  }
}

void RegisterScene::CreateUIElements() {
  SDL_Color white_color{255, 255, 255, 255};

  title_text_ = std::make_unique<Text>(
      100, 40, "Register:", font_, white_color, renderer_.GetSDLRenderer());

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

  confirm_password_label_ = std::make_unique<Text>(
      100, 220, "Confirm Password:", font_, white_color, renderer_.GetSDLRenderer());

  confirm_password_box_ = std::make_unique<TextBox>(
      230, 215, 400, 40,
      std::make_unique<Text>(
          0, 0, "", font_, white_color, renderer_.GetSDLRenderer()), 32);

  auto register_text = std::make_unique<Text>(
      0, 0, "Register", font_, SDL_Color{255, 255, 255, 255}, renderer_.GetSDLRenderer());

  register_button_ = std::make_unique<TextButton>(
      100, 280, 150, 50, std::move(register_text));

  register_button_->SetRenderStrategy(ButtonState::Normal, [](SDL_Renderer* renderer_, const Button& button) {
    SDL_SetRenderDrawColor(renderer_, 0, 128, 0, 255);
    SDL_RenderFillRect(renderer_, &button.GetBounds());
  });

  register_button_->SetRenderStrategy(ButtonState::Hover, [](SDL_Renderer* renderer_, const Button& button) {
    SDL_SetRenderDrawColor(renderer_, 0, 255, 0, 255);
    SDL_RenderFillRect(renderer_, &button.GetBounds());
  });

  register_button_->SetOnClick([this]() { OnRegisterButtonClicked(); });

  status_text_ = std::make_unique<Text>(
      100, 350, "", font_, SDL_Color{255, 0, 0, 255}, renderer_.GetSDLRenderer());

  login_link_ = std::make_unique<TextLink>(
      100, 400, "Go back to Login.", font_, SDL_Color{0, 0, 255, 255}, SDL_Color{128, 0, 255, 255},
      renderer_.GetSDLRenderer());

  login_link_->SetOnClick([this]() {
    std::cout << "[RegisterScene] Login link clicked. Transitioning to LoginScene." << std::endl;
    scene_manager_.ReplaceScene(std::make_unique<LoginScene>());
  });
}

void RegisterScene::OnRegisterButtonClicked() {
  if (is_waiting_) return;

  const std::string username = username_box_->GetContent();
  const std::string password = password_box_->GetContent();
  const std::string confirm_password = confirm_password_box_->GetContent();

  if (username.empty() || password.empty() || confirm_password.empty()) {
    status_text_->SetContent("Error: All fields must be filled.");
    return;
  }

  if (password != confirm_password) {
    status_text_->SetContent("Error: Passwords do not match.");
    return;
  }

  is_waiting_ = true;

  std::cout << "[RegisterScene] Registration attempt: username='" << username << "'" << std::endl;

  if (auto register_packet = network::CreateRegisterPacket<network::MyPacketType>(username, password)) {
    network_server_.SendTcp(std::move(*register_packet));
  } else {
    status_text_->SetContent("Error: Failed to create register packet.");
    is_waiting_ = false;
  }
}

void RegisterScene::HandleRegisterResponse(const network::packets::RegisterResponsePacket& packet) {
  if (packet.status_code == 200) {
    std::cout << "[RegisterScene] Registration successful. Redirecting to LoginScene." << std::endl;
    status_text_->SetContent("Registration successful! You can login now.");
    status_text_->SetColor(SDL_Color{0, 255, 0, 255});
  } else {
    std::cout << "[RegisterScene] Registration failed. Status code: " << packet.status_code << std::endl;
    status_text_->SetContent("Registration failed. Please try again.");
    is_waiting_ = false;
  }
}

void RegisterScene::Enter() {
  std::cout << "[RegisterScene] Enter()" << std::endl;
  SDL_StartTextInput();

  event_queue_.Subscribe(EventType::RegisterResponse, [this](const network::Packet<network::MyPacketType>& packet) {
    if (packet.body.size() != sizeof(network::packets::RegisterResponsePacket)) {
      status_text_->SetContent("Error: Invalid server response.");
      is_waiting_ = false;
      return;
    }
    const auto* response = reinterpret_cast<const network::packets::RegisterResponsePacket*>(packet.body.data());
    HandleRegisterResponse(*response);
  });
}

void RegisterScene::Exit() {
  std::cout << "[RegisterScene] Exit()" << std::endl;
  SDL_StopTextInput();
  event_queue_.ClearHandlers(EventType::RegisterResponse);
}

void RegisterScene::Update(float /*delta_time*/) {}

void RegisterScene::Render() {
  if (title_text_) title_text_->Render(renderer_.GetSDLRenderer());
  if (username_label_) username_label_->Render(renderer_.GetSDLRenderer());
  if (username_box_) username_box_->Render(renderer_.GetSDLRenderer());
  if (password_label_) password_label_->Render(renderer_.GetSDLRenderer());
  if (password_box_) password_box_->Render(renderer_.GetSDLRenderer());
  if (confirm_password_label_) confirm_password_label_->Render(renderer_.GetSDLRenderer());
  if (confirm_password_box_) confirm_password_box_->Render(renderer_.GetSDLRenderer());
  if (register_button_) register_button_->Render(renderer_.GetSDLRenderer());
  if (status_text_) status_text_->Render(renderer_.GetSDLRenderer());
  if (login_link_) login_link_->Render(renderer_.GetSDLRenderer());
}

void RegisterScene::HandleInput(const SDL_Event& e) {
  if (username_box_) username_box_->HandleInput(e);
  if (password_box_) password_box_->HandleInput(e);
  if (confirm_password_box_) confirm_password_box_->HandleInput(e);
  if (register_button_) register_button_->HandleInput(e);
  if (login_link_) login_link_->HandleInput(e);
}

}  // namespace rtype
