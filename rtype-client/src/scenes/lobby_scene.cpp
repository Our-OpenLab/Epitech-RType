
#include <iostream>

#include "scenes/game_scene.hpp"
#include "scenes/main_menu_scene.hpp"
#include "scenes/lobby_scene.hpp"

namespace rtype {

LobbyScene::LobbyScene(const int lobby_id)
    : lobby_id_(lobby_id),
      renderer_(ServiceLocator::Get<Renderer>()),
      scene_manager_(ServiceLocator::Get<SceneManager>()),
      event_queue_(ServiceLocator::Get<EventQueue<network::Packet<network::MyPacketType>>>()),
      network_server_(ServiceLocator::Get<network::NetworkClient<network::MyPacketType>>()) {
  InitializeUI();
}

void LobbyScene::InitializeUI() {
  const std::string font_path = "assets/fonts/Roboto-Regular.ttf";
  constexpr int font_size = 24;
  SDL_Color white_color{255, 255, 255, 255};

  if (!GlobalResourceManager::Instance().LoadFont(font_path, font_path, font_size)) {
    throw std::runtime_error("[LobbyScene] Failed to load font: Roboto-Regular.");
  }

  font_ = GlobalResourceManager::Instance().GetFont(font_path);
  if (!font_) {
    throw std::runtime_error("[LobbyScene] Font was loaded but could not be retrieved.");
  }

  // Title
  title_text_ = std::make_unique<Text>(
      100, 40, "Lobby ID: " + std::to_string(lobby_id_), font_, white_color, renderer_.GetSDLRenderer());

  // Player list title
  player_list_title_ = std::make_unique<Text>(
      100, 100, "Players in Lobby:", font_, white_color, renderer_.GetSDLRenderer());

  // Ready/Not Ready Button
  auto ready_button_text = std::make_unique<Text>(
      0, 0, "Not Ready", font_, white_color, renderer_.GetSDLRenderer());
  ready_button_ = std::make_unique<TextButton>(
      100, 300, 200, 50, std::move(ready_button_text));
  ready_button_->SetRenderStrategy(ButtonState::Normal, [](SDL_Renderer* renderer_, const Button& button) {
    SDL_SetRenderDrawColor(renderer_, 0, 128, 255, 255);  // Blue
    SDL_RenderFillRect(renderer_, &button.GetBounds());
  });
  ready_button_->SetRenderStrategy(ButtonState::Hover, [](SDL_Renderer* renderer_, const Button& button) {
    SDL_SetRenderDrawColor(renderer_, 0, 255, 255, 255);  // Bright Blue
    SDL_RenderFillRect(renderer_, &button.GetBounds());
  });
  ready_button_->SetOnClick([this]() { OnReadyButtonClicked(); });

  // Leave Lobby Button
  auto leave_lobby_text = std::make_unique<Text>(
      0, 0, "Leave Lobby", font_, white_color, renderer_.GetSDLRenderer());
  leave_lobby_button_ = std::make_unique<TextButton>(
      100, 370, 200, 50, std::move(leave_lobby_text));
  leave_lobby_button_->SetRenderStrategy(ButtonState::Normal, [](SDL_Renderer* renderer_, const Button& button) {
    SDL_SetRenderDrawColor(renderer_, 128, 0, 0, 255);  // Red
    SDL_RenderFillRect(renderer_, &button.GetBounds());
  });
  leave_lobby_button_->SetRenderStrategy(ButtonState::Hover, [](SDL_Renderer* renderer_, const Button& button) {
    SDL_SetRenderDrawColor(renderer_, 255, 0, 0, 255);  // Bright Red
    SDL_RenderFillRect(renderer_, &button.GetBounds());
  });
  leave_lobby_button_->SetOnClick([this]() { OnLeaveLobbyButtonClicked(); });
}

void LobbyScene::OnReadyButtonClicked() {
  is_ready_ = !is_ready_;

  // Change local UI state
  const std::string button_text = is_ready_ ? "Ready" : "Not Ready";
  ready_button_->SetText(button_text);

  // Envoie le paquet de readiness au serveur
  auto ready_packet = network::CreatePlayerReadyPacket<network::MyPacketType>(is_ready_);
  if (ready_packet) {
    network_server_.SendTcp(std::move(*ready_packet));
    std::cout << "[LobbyScene] Player readiness update requested: " << (is_ready_ ? "Ready" : "Not Ready") << "." << std::endl;
  } else {
    std::cerr << "[LobbyScene][ERROR] Failed to create PlayerReadyPacket." << std::endl;
  }
}

void LobbyScene::OnLeaveLobbyButtonClicked() {
  std::cout << "[LobbyScene] Leave Lobby button clicked for Lobby ID: " << lobby_id_ << "." << std::endl;

  auto leave_lobby_packet = network::CreateLeaveLobbyPacket<network::MyPacketType>();

  if (!leave_lobby_packet) {
    std::cerr << "[LobbyScene][ERROR] Failed to create LeaveLobbyPacket." << std::endl;
    return;
  }

  network_server_.SendTcp(std::move(*leave_lobby_packet));

  // S'abonner à la réponse LeaveLobbyResponse
}

void LobbyScene::HandlePlayerReadyResponse(const network::Packet<network::MyPacketType>& packet) {
  if (packet.body.size() < sizeof(int)) {
    std::cerr << "[LobbyScene][ERROR] Invalid PlayerReadyResponsePacket size." << std::endl;
    return;
  }

  if (const auto* response =
          reinterpret_cast<const network::packets::PlayerReadyPacketResponse*>(
              packet.body.data());
      response->status_code == 200) {
    std::cout << "[LobbyScene] Player readiness successfully updated: " << (is_ready_ ? "Ready" : "Not Ready") << "." << std::endl;
  } else {
    std::cerr << "[LobbyScene][ERROR] Failed to update readiness on server. Code: " << response->status_code << std::endl;
    // Revert local state since server did not accept the change
    is_ready_ = !is_ready_;
    const std::string button_text = is_ready_ ? "Ready" : "Not Ready";
    ready_button_->SetText(button_text);
  }
}

void LobbyScene::HandleLeaveLobbyResponse(const network::Packet<network::MyPacketType>& packet) {
  const auto response_code_opt = network::PacketFactory<network::MyPacketType>::ExtractData<int>(packet);

  if (!response_code_opt) {
    std::cerr << "[LobbyScene][ERROR] Failed to extract response code from packet." << std::endl;
    return;
  }

  const int response_code = *response_code_opt;

  if (response_code == 200) {
    std::cout << "[LobbyScene] Successfully left the lobby." << std::endl;
    scene_manager_.ReplaceScene(std::make_unique<MainMenuScene>());
  } else {
    std::cerr << "[LobbyScene][ERROR] Failed to leave the lobby. Code: " << response_code << std::endl;
  }
}

void LobbyScene::Enter() {
  std::cout << "[LobbyScene] Entered with Lobby ID: " << lobby_id_ << "." << std::endl;

  // Subscribe to GetLobbyPlayersResponse event
  event_queue_.Subscribe(EventType::GetLobbyPlayersResponse, [this](const network::Packet<network::MyPacketType>& packet) {
    HandleGetLobbyPlayersResponse(packet);
  });

  // Subscribe to PlayerJoinedLobby event
  event_queue_.Subscribe(EventType::PlayerJoinedLobby, [this](const network::Packet<network::MyPacketType>& packet) {
    HandlePlayerJoinedLobby(packet);
  });

  // Subscribe to PlayerLeftLobby event
  event_queue_.Subscribe(EventType::PlayerLeftLobby, [this](const network::Packet<network::MyPacketType>& packet) {
    HandlePlayerLeftLobby(packet);
  });

  event_queue_.Subscribe(EventType::LeaveLobbyResponse, [this](const network::Packet<network::MyPacketType>& packet) {
    HandleLeaveLobbyResponse(packet);
  });

  event_queue_.Subscribe(EventType::PlayerReadyResponse, [this](const network::Packet<network::MyPacketType>& packet) {
    HandlePlayerReadyResponse(packet);
  });

  event_queue_.Subscribe(EventType::LobbyPlayerReady, [this](const network::Packet<network::MyPacketType>& packet) {
    HandlePlayerReady(packet);
  });

  event_queue_.Subscribe(EventType::GameConnectionInfo, [this](const network::Packet<network::MyPacketType>& packet) {
    HandleGameConnectionInfo(packet);
  });

  // Send request to get players in the lobby
  auto packet = network::CreateGetLobbyPlayersPacket<network::MyPacketType>(lobby_id_);
  if (packet) {
    network_server_.SendTcp(std::move(*packet));
  } else {
    std::cerr << "[LobbyScene][ERROR] Failed to create GetLobbyPlayersPacket." << std::endl;
  }
}

void LobbyScene::HandlePlayerReady(const network::Packet<network::MyPacketType>& packet) {
  if (packet.body.size() != sizeof(network::packets::LobbyPlayerReadyPacket)) {
    std::cerr << "[LobbyScene][ERROR] Invalid LobbyPlayerReadyPacket size." << std::endl;
    return;
  }

  const auto* notification = reinterpret_cast<const network::packets::LobbyPlayerReadyPacket*>(packet.body.data());
  const int player_id = notification->player_id;
  const bool is_ready = notification->is_ready;

  auto it = player_map_.find(player_id);
  if (it != player_map_.end()) {
    // Mettre à jour le texte du joueur
    std::string updated_text = it->second->GetContent();
    updated_text = updated_text.substr(0, updated_text.find(" (")) + (is_ready ? " (Ready)" : " (Not Ready)");
    it->second->SetContent(updated_text);

    std::cout << "[LobbyScene] Updated readiness for player ID " << player_id
              << ": " << (is_ready ? "Ready" : "Not Ready") << "." << std::endl;
  } else {
    std::cerr << "[LobbyScene][WARNING] Player ID " << player_id << " not found in the lobby map." << std::endl;
  }
}

void LobbyScene::Exit() {
  std::cout << "[LobbyScene] Exit()" << std::endl;

  event_queue_.ClearHandlers(EventType::GetLobbyPlayersResponse);
  event_queue_.ClearHandlers(EventType::PlayerJoinedLobby);
  event_queue_.ClearHandlers(EventType::PlayerLeftLobby);
  event_queue_.ClearHandlers(EventType::LeaveLobbyResponse);
  event_queue_.ClearHandlers(EventType::PlayerReadyResponse);
  event_queue_.ClearHandlers(EventType::GameConnectionInfo);
}


void LobbyScene::Update(float /*delta_time*/) {}

void LobbyScene::Render() {
  if (title_text_) title_text_->Render(renderer_.GetSDLRenderer());
  if (player_list_title_) player_list_title_->Render(renderer_.GetSDLRenderer());

  int index = 0;
  for (const auto& [player_id, player_text] : player_map_) {
    player_text->SetPosition(100, 140 + index * 30);
    player_text->Render(renderer_.GetSDLRenderer());
    ++index;
  }

  if (ready_button_) ready_button_->Render(renderer_.GetSDLRenderer());
  if (leave_lobby_button_) leave_lobby_button_->Render(renderer_.GetSDLRenderer());
}

void LobbyScene::HandleInput(const SDL_Event& event) {
  if (ready_button_) ready_button_->HandleInput(event);
  if (leave_lobby_button_) leave_lobby_button_->HandleInput(event);
}

void LobbyScene::HandleGetLobbyPlayersResponse(const network::Packet<network::MyPacketType>& packet) {
  if (packet.body.size() < sizeof(int)) {
    std::cerr << "[LobbyScene][ERROR] Invalid GetLobbyPlayersResponsePacket size." << std::endl;
    return;
  }

  const int status_code = *reinterpret_cast<const int*>(packet.body.data());
  if (status_code != 200) {
    std::cerr << "[LobbyScene][ERROR] Failed to retrieve players. Status code: " << status_code << std::endl;
    return;
  }

  constexpr size_t players_offset = sizeof(int);
  const size_t players_data_size = packet.body.size() - players_offset;

  if (players_data_size % sizeof(network::packets::GetLobbyPlayersResponsePacket::PlayerInfo) != 0) {
    std::cerr << "[LobbyScene][ERROR] Invalid player data size in GetLobbyPlayersResponsePacket." << std::endl;
    return;
  }

  const size_t player_count = players_data_size / sizeof(network::packets::GetLobbyPlayersResponsePacket::PlayerInfo);
  const auto* players = reinterpret_cast<const network::packets::GetLobbyPlayersResponsePacket::PlayerInfo*>(
      packet.body.data() + players_offset);

  player_map_.clear();

  for (size_t i = 0; i < player_count; ++i) {
    std::string player_text_content = std::string(players[i].username) +
                                       (players[i].is_ready ? " (Ready)" : " (Not Ready)");

    auto player_text = std::make_unique<Text>(
        100, 140 + static_cast<int>(i) * 30,
        player_text_content,
        font_, SDL_Color{255, 255, 255, 255}, renderer_.GetSDLRenderer());
    player_map_.emplace(players[i].player_id, std::move(player_text));

    std::cout << "[LobbyScene] Player: " << players[i].username
              << " (ID: " << players[i].player_id
              << ", Ready: " << (players[i].is_ready ? "Yes" : "No") << ")" << std::endl;
  }
}

void LobbyScene::HandlePlayerJoinedLobby(const network::Packet<network::MyPacketType>& packet) {
  if (packet.body.size() != sizeof(network::packets::PlayerJoinedLobbyPacket)) {
    std::cerr << "[LobbyScene][ERROR] Invalid PlayerJoinedLobbyPacket size." << std::endl;
    return;
  }

  const auto* join_data = reinterpret_cast<const network::packets::PlayerJoinedLobbyPacket*>(packet.body.data());

  if (player_map_.find(join_data->player_id) != player_map_.end()) {
    std::cerr << "[LobbyScene][WARNING] Player already in lobby: ID " << join_data->player_id << "." << std::endl;
    return;
  }

  std::string player_text_content = std::string(join_data->username) + " (Not Ready)";

  auto player_text = std::make_unique<Text>(
      100, 140 + static_cast<int>(player_map_.size()) * 30,
      player_text_content,
      font_, SDL_Color{255, 255, 255, 255}, renderer_.GetSDLRenderer());

  player_map_.emplace(join_data->player_id, std::move(player_text));

  std::cout << "[LobbyScene] Player joined: " << join_data->username
            << " (ID: " << join_data->player_id << ", Ready: No)." << std::endl;
}

void LobbyScene::HandlePlayerLeftLobby(const network::Packet<network::MyPacketType>& packet) {
  if (packet.body.size() != sizeof(network::packets::PlayerLeftLobbyPacket)) {
    std::cerr << "[LobbyScene][ERROR] Invalid PlayerLeftLobbyPacket size." << std::endl;
    return;
  }

  const auto* leave_data = reinterpret_cast<const network::packets::PlayerLeftLobbyPacket*>(packet.body.data());

  if (player_map_.erase(leave_data->player_id) == 0) {
    std::cerr << "[LobbyScene][WARNING] Player not found in lobby: ID " << leave_data->player_id << "." << std::endl;
    return;
  }

  std::cout << "[LobbyScene] Player left: ID " << leave_data->player_id << "." << std::endl;
}

void LobbyScene::HandleGameConnectionInfo(const network::Packet<network::MyPacketType>& packet) {
  if (packet.body.size() != sizeof(network::packets::GameConnectionInfoPacket)) {
    std::cerr << "[LobbyScene][ERROR] Invalid GameConnectionInfoPacket size." << std::endl;
    return;
  }

  const auto* connection_info = reinterpret_cast<const network::packets::GameConnectionInfoPacket*>(packet.body.data());

  std::string ip_address(connection_info->ip_address, sizeof(connection_info->ip_address));
  std::erase(ip_address, '\0');

  std::vector ports(std::begin(connection_info->ports), std::end(connection_info->ports));
  std::erase(ports, 0);

  if (ip_address.empty() || ports.empty()) {
    std::cerr << "[LobbyScene][ERROR] Invalid game connection info received." << std::endl;
    return;
  }

  std::cout << "[LobbyScene] Game connection info received. IP: " << ip_address << ", Ports: ";
  for (const auto& port : ports) {
    std::cout << port << " ";
  }
  std::cout << std::endl;

  scene_manager_.ReplaceScene(std::make_unique<GameScene>(ip_address, ports));
}


}  // namespace rtype
