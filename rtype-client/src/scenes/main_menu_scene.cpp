#include "scenes/main_menu_scene.hpp"

#include "core/resource_manager.hpp"
#include "core/service_locator.hpp"
#include "scenes/create_lobby_scene.hpp"
#include "scenes/lobby_password_scene.hpp"
#include "scenes/lobby_scene.hpp"

namespace rtype {

MainMenuScene::MainMenuScene()
    : lobby_map_(std::make_shared<std::unordered_map<
                     int, std::pair<std::unique_ptr<Text>,
                                    std::unique_ptr<TextButton>>>>()),
      renderer_(ServiceLocator::Get<Renderer>("renderer")),
      scene_manager_(ServiceLocator::Get<SceneManager>("scene_manager")),
      event_queue_(ServiceLocator::Get<
                   EventQueue<network::Packet<network::MyPacketType>>>("event_queue")),
      network_server_(ServiceLocator::Get<
                      network::NetworkClient<network::MyPacketType>>("network_server")) {
  if (ServiceLocator::Has<ChatOverlay>("chat_overlay")) {
    chat_overlay_ = ServiceLocator::GetShared<ChatOverlay>("chat_overlay");
  } else {
    chat_overlay_ = std::make_shared<ChatOverlay>();
    ServiceLocator::Provide("chat_overlay", chat_overlay_);
  }

  InitializeUI();
}

void MainMenuScene::Enter() {
  std::cout << "[MainMenuScene] Enter()" << std::endl;
  SDL_StartTextInput();

  event_queue_.Subscribe(EventType::GetLobbyListResponse, [this](const network::Packet<network::MyPacketType>& packet) {
    HandleLobbiesResponse(packet);
  });

  event_queue_.Subscribe(EventType::JoinLobbyResponse, [this](const network::Packet<network::MyPacketType>& packet) {
    if (packet.body.size() != sizeof(network::packets::JoinLobbyResponsePacket)) {
     // status_text_->SetContent("Error: Invalid server response.");
      return;
    }
    const auto* response = reinterpret_cast<const network::packets::JoinLobbyResponsePacket*>(packet.body.data());
    HandleJoinLobbyResponse(*response);
  });

  RequestLobbies();
}

void MainMenuScene::HandleJoinLobbyResponse(const network::packets::JoinLobbyResponsePacket& packet) {
  if (packet.status_code == 200) {
  //  status_text_->SetContent("Joined lobby successfully!");
  //  status_text_->SetColor(SDL_Color{0, 255, 0, 255});
    //std::cout << "[LobbyPasswordScene] Successfully joined lobby. Transitioning to LobbyScene." << std::endl;

    // Transition to LobbyScene
    scene_manager_.ReplaceScene(std::make_unique<LobbyScene>(lobby_id_));
  } else {
 //   status_text_->SetContent("Error: Incorrect password.");
  //  std::cout << "[LobbyPasswordScene] Failed to join lobby. Status code: " << packet.status_code << std::endl;
  }
}

void MainMenuScene::Exit() {
  std::cout << "[MainMenuScene] Exit()" << std::endl;
  SDL_StopTextInput();
  event_queue_.ClearHandlers(EventType::GetLobbyListResponse);
  event_queue_.ClearHandlers(EventType::JoinLobbyResponse);
}

void MainMenuScene::Update(float /*delta_time*/) {
  // Logic update (if necessary)
}

void MainMenuScene::Render() {
  if (title_text_) title_text_->Render(renderer_.GetSDLRenderer());
  if (search_title_) search_title_->Render(renderer_.GetSDLRenderer());
  if (play_button_) play_button_->Render(renderer_.GetSDLRenderer());
  if (settings_button_) settings_button_->Render(renderer_.GetSDLRenderer());
  if (exit_button_) exit_button_->Render(renderer_.GetSDLRenderer());
  if (next_page_button_) next_page_button_->Render(renderer_.GetSDLRenderer());
  if (prev_page_button_) prev_page_button_->Render(renderer_.GetSDLRenderer());
  if (refresh_button_) refresh_button_->Render(renderer_.GetSDLRenderer());

  if (search_box_) search_box_->Render(renderer_.GetSDLRenderer());

  // Render lobbies and their join buttons
  if (lobby_map_) {
    for (const auto& [id, pair] : *lobby_map_) {
      pair.first->Render(renderer_.GetSDLRenderer());  // Render text
      pair.second->Render(renderer_.GetSDLRenderer()); // Render button
    }
  }

  if (info_text_) info_text_->Render(renderer_.GetSDLRenderer());

  if (chat_overlay_) chat_overlay_->Render(renderer_.GetSDLRenderer());
}


void MainMenuScene::HandleInput(const SDL_Event& event) {
  if (play_button_) play_button_->HandleInput(event);
  if (settings_button_) settings_button_->HandleInput(event);
  if (exit_button_) exit_button_->HandleInput(event);
  if (next_page_button_) next_page_button_->HandleInput(event);
  if (prev_page_button_) prev_page_button_->HandleInput(event);
  if (refresh_button_) refresh_button_->HandleInput(event);

  if (search_box_) search_box_->HandleInput(event);

  if (const auto lobby_map_snapshot = lobby_map_) {
    for (auto it = lobby_map_snapshot->begin(); it != lobby_map_snapshot->end(); ++it) {
      auto& [fst, snd] = it->second;
      if (!lobby_map_)
        return;
      if (snd) {
        snd->HandleInput(event);
      }
    }
  }

  if (chat_overlay_) chat_overlay_->HandleInput(event);
}


void MainMenuScene::InitializeUI() {
  const std::string font_path = "assets/fonts/Roboto-Regular.ttf";
  constexpr int font_size = 24;
  SDL_Color white_color{255, 255, 255, 255};

  // Load font
  if (!GlobalResourceManager::Instance().LoadFont(font_path, font_path, font_size)) {
    throw std::runtime_error("[MainMenuScene] Failed to load font: Roboto-Regular.");
  }
  font_ = GlobalResourceManager::Instance().GetFont(font_path);
  if (!font_) {
    throw std::runtime_error("[MainMenuScene] Font was loaded but could not be retrieved.");
  }

  title_text_ = std::make_unique<Text>(
      850, 50, "Main Menu", font_, white_color, renderer_.GetSDLRenderer());
  search_title_ = std::make_unique<Text>(
      100, 150, "Search Lobbies:", font_, white_color, renderer_.GetSDLRenderer());


  auto search_text = std::make_unique<Text>(
      0, 0, "", font_, white_color, renderer_.GetSDLRenderer());
  search_box_ = std::make_unique<TextBox>(
      100, 200, 400, 50, std::move(search_text), 32);

  refresh_button_ = std::make_unique<TextButton>(520, 200, 120, 50, std::make_unique<Text>(
      0, 0, "Refresh", font_, white_color, renderer_.GetSDLRenderer()));
  refresh_button_->SetRenderStrategy(ButtonState::Normal, [](SDL_Renderer* renderer, const Button& button) {
    SDL_SetRenderDrawColor(renderer, 0, 128, 255, 255);
    SDL_RenderFillRect(renderer, &button.GetBounds());
  });
  refresh_button_->SetOnClick([this]() {
    current_page_ = 0;
    RequestLobbies();
  });

  next_page_button_ = std::make_unique<TextButton>(850, 600, 120, 50, std::make_unique<Text>(
      0, 0, "Next", font_, white_color, renderer_.GetSDLRenderer()));
  next_page_button_->SetOnClick([this]() { current_page_++; RequestLobbies(); });

  prev_page_button_ = std::make_unique<TextButton>(700, 600, 120, 50, std::make_unique<Text>(
      0, 0, "Previous", font_, white_color, renderer_.GetSDLRenderer()));
  prev_page_button_->SetOnClick([this]() {
    if (current_page_ > 0) {
      current_page_--;
      RequestLobbies();
    }
  });

  // Create Lobby Button
  auto create_lobby_text = std::make_unique<Text>(
      0, 0, "Create Lobby", font_, white_color, renderer_.GetSDLRenderer());
  play_button_ = std::make_unique<TextButton>(
      850, 150, 200, 50, std::move(create_lobby_text));
  play_button_->SetRenderStrategy(ButtonState::Normal, [](SDL_Renderer* renderer, const Button& button) {
    SDL_SetRenderDrawColor(renderer, 0, 128, 0, 255);  // Green
    SDL_RenderFillRect(renderer, &button.GetBounds());
  });
  play_button_->SetRenderStrategy(ButtonState::Hover, [](SDL_Renderer* renderer, const Button& button) {
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);  // Bright Green
    SDL_RenderFillRect(renderer, &button.GetBounds());
  });
  play_button_->SetOnClick([this]() { OnCreateLobbyButtonClicked(); });

  // Settings Button
  auto settings_text = std::make_unique<Text>(
      0, 0, "Settings", font_, white_color, renderer_.GetSDLRenderer());
  settings_button_ = std::make_unique<TextButton>(
      850, 220, 200, 50, std::move(settings_text));
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
      850, 290, 200, 50, std::move(exit_text));
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

void MainMenuScene::OnCreateLobbyButtonClicked() {
  std::cout << "[MainMenuScene] Create Lobby button clicked. Transitioning to CreateLobbyScene." << std::endl;
  scene_manager_.ReplaceScene(std::make_unique<CreateLobbyScene>());
}

void MainMenuScene::OnSettingsButtonClicked() {
  std::cout << "[MainMenuScene] Settings button clicked. Transitioning to SettingsScene." << std::endl;
  // Transition logic here
}

void MainMenuScene::OnExitButtonClicked() {
  std::cout << "[MainMenuScene] Exit button clicked. Exiting application." << std::endl;
  SDL_Quit();
  exit(0);
}

void MainMenuScene::HandleLobbiesResponse(const network::Packet<network::MyPacketType>& packet) {
    if (packet.body.size() < sizeof(int)) {
        std::cerr << "[MainMenuScene][ERROR] Invalid LobbiesResponsePacket size." << std::endl;
        return;
    }

    const int status_code = *reinterpret_cast<const int*>(packet.body.data());

    if (status_code == 404) {
        std::cerr << "[MainMenuScene][INFO] No lobbies found." << std::endl;
        if (lobby_map_) {
            lobby_map_->clear();
        }

        info_text_ = std::make_unique<Text>(
            100, 300, "No lobbies found.", font_, SDL_Color{255, 255, 255, 255}, renderer_.GetSDLRenderer());
        return;
    }

    if (status_code != 200) {
        std::cerr << "[MainMenuScene][ERROR] Failed to retrieve lobbies. Status code: " << status_code << std::endl;
        return;
    }

    info_text_.reset();

    constexpr size_t lobbies_offset = sizeof(int);
    const size_t lobbies_data_size = packet.body.size() - lobbies_offset;

    if (lobbies_data_size % sizeof(network::packets::GetLobbyListResponsePacket::LobbyInfo) != 0) {
        std::cerr << "[MainMenuScene][ERROR] Invalid lobby data size in LobbiesResponsePacket." << std::endl;
        return;
    }

    const size_t lobby_count = lobbies_data_size / sizeof(network::packets::GetLobbyListResponsePacket::LobbyInfo);
    const auto* lobbies = reinterpret_cast<const network::packets::GetLobbyListResponsePacket::LobbyInfo*>(
        packet.body.data() + lobbies_offset);

    if (!lobby_map_) {
        lobby_map_ = std::make_unique<std::unordered_map<int, std::pair<std::unique_ptr<Text>, std::unique_ptr<TextButton>>>>();
    }

    lobby_map_->clear();

    for (size_t i = 0; i < lobby_count; ++i) {
        // Create the lobby text
        auto lobby_text = std::make_unique<Text>(
            100, 300 + static_cast<int>(i) * 60,  // Adjust Y offset for spacing
            lobbies[i].name, font_, SDL_Color{255, 255, 255, 255}, renderer_.GetSDLRenderer());

        // Create the join button
        auto join_button = std::make_unique<TextButton>(
            400, 300 + static_cast<int>(i) * 60, 120, 40,  // Position near the text
            std::make_unique<Text>(0, 0, "Join", font_, SDL_Color{255, 255, 255, 255}, renderer_.GetSDLRenderer()));

        join_button->SetRenderStrategy(ButtonState::Normal, [](SDL_Renderer* renderer, const Button& button) {
            SDL_SetRenderDrawColor(renderer, 0, 128, 0, 255);  // Green
            SDL_RenderFillRect(renderer, &button.GetBounds());
        });

        join_button->SetOnClick([this, lobby_id = lobbies[i].lobby_id, has_password = lobbies[i].has_password]() {
          lobby_id_ = lobby_id;
          if (has_password) {
              // Change to a password input scene
              std::cout << "[MainMenuScene] Lobby requires a password. Transitioning to password input scene." << std::endl;
              scene_manager_.ReplaceScene(std::make_unique<LobbyPasswordScene>(lobby_id));
          } else {
              // Send request to join the lobby
              std::cout << "[MainMenuScene] Joining lobby ID: " << lobby_id << std::endl;
              network_server_.SendTcp(std::move(*network::CreateJoinLobbyPacket<network::MyPacketType>(lobby_id, "")));
          }
        });

        // Add the text and button as a pair to the map
        (*lobby_map_)[lobbies[i].lobby_id] = std::make_pair(std::move(lobby_text), std::move(join_button));

        // Log the lobby information
        std::cout << "[MainMenuScene] Lobby: " << lobbies[i].name
                  << " (ID: " << lobbies[i].lobby_id
                  << ", Password Protected: " << (lobbies[i].has_password ? "Yes" : "No") << ")" << std::endl;
    }
}

void MainMenuScene::RequestLobbies() {
  const std::string search_term = search_box_ ? search_box_->GetContent() : "";
  auto packet = network::CreateGetLobbyListPacket<network::MyPacketType>(
      current_page_ * 10, 10, search_term);
  if (packet) {
    network_server_.SendTcp(std::move(*packet));
  }
}

}  // namespace rtype
