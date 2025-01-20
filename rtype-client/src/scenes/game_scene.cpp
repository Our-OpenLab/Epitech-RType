#include "scenes/game_scene.hpp"

#include <iostream>
#include <utility>

#include "core/packet_factory.hpp"
#include "core/resource_manager.hpp"
#include "core/protocol.hpp"

namespace rtype {

GameScene::GameScene(std::string ip_address, const std::vector<int>& ports)
    : ip_address_(std::move(ip_address)),
      ports_(ports),
      is_connected_(false),
      renderer_(ServiceLocator::Get<Renderer>()),
      scene_manager_(ServiceLocator::Get<SceneManager>()),
      event_queue_(ServiceLocator::Get<
                   EventQueue<network::Packet<network::MyPacketType>>>()),
      network_server_(
          ServiceLocator::Get<network::NetworkClient<network::MyPacketType>>()),
      input_manager_(
          [this](InputManager::PlayerInput&& input) {
            HandlePlayerInput(std::move(input));
          },
          screen_manager_),
      game_state_(registry_)
{
  InitializeUI();

  registry_.register_component<ClientPlayer>();
  registry_.register_component<Enemy>();
  registry_.register_component<Projectile>();
  registry_.register_component<Position>();
  // ConnectToServer();
}

void GameScene::InitializeUI() {
//  const std::string font_path = "assets/fonts/Roboto-Regular.ttf";
//  constexpr int font_size = 24;
//  SDL_Color white_color{255, 255, 255, 255};

//  if (!GlobalResourceManager::Instance().LoadFont(font_path, font_path, font_size)) {
//    throw std::runtime_error("[GameScene] Failed to load font: Roboto-Regular.");
//  }

 // font_ = GlobalResourceManager::Instance().GetFont(font_path);
 // if (!font_) {
 //   throw std::runtime_error("[GameScene] Font was loaded but could not be retrieved.");
 // }

 // connection_status_text_ = std::make_unique<Text>(
 //     100, 100, "Connecting to game server...", font_, white_color, renderer_.GetSDLRenderer());
}

void GameScene::ConnectToServer() {
  if (ports_.empty()) {
    throw std::runtime_error("[GameScene][ERROR] No ports provided for game connection.");
  }

  if (network_server_.Connect(ip_address_, std::to_string(ports_[0]), ports_[1])) {
    std::cout << "[GameScene][INFO] Successfully connected to server at " << ip_address_
              << " on ports TCP: " << ports_[0] << ", UDP: " << ports_[1] << std::endl;
    is_connected_ = true;
  } else {
    std::cerr << "[GameScene][ERROR] Failed to connect to server at " << ip_address_
              << " on ports TCP: " << ports_[0] << ", UDP: " << ports_[1] << std::endl;
    throw std::runtime_error("Failed to connect to game server.");
  }
}

void GameScene::Enter() {
  std::cout << "[GameScene] Enter()" << std::endl;

  event_queue_.Subscribe(EventType::PlayerAssign, [this](const network::Packet<network::MyPacketType>& packet) {
    const auto extracted_data =
        network::PacketFactory<network::MyPacketType>::ExtractData<network::packets::PlayerAssign>(packet);

    if (!extracted_data) {
        std::cerr << "[Client][ERROR] Failed to extract PlayerAssign data from packet: invalid size." << std::endl;
        return;
    }

    const auto& [spawn_x, spawn_y, score, player_id, health] = *extracted_data;

    SetClientId(player_id);

    const auto entity = game_state_.AddPlayer(player_id, spawn_x, spawn_y, score, health);

    if (entity == client::GameState::InvalidEntity) {
        std::cerr << "[Server][ERROR] Failed to add Player ID: " << static_cast<int>(player_id)
                  << " to GameState. Player might already exist." << std::endl;
        return;
    }

    game_state_.SetLocalPlayerEntity(entity);

    const auto udp_port = network_server_.GetLocalUdpPort();
    if (udp_port == 0) {
        std::cerr << "[Client][ERROR] Invalid UDP port. Cannot send to server." << std::endl;
        return;
    }

    // Récupération de l'adresse IP privée
    std::string local_ip;
    try {
      asio::ip::tcp::resolver resolver(network_server_.GetIoContext());
      const auto endpoints = resolver.resolve(asio::ip::host_name(), "");
      for (const auto& endpoint : endpoints) {
        const auto& address = endpoint.endpoint().address();
        if (address.is_v4() && !address.is_loopback()) {
            local_ip = address.to_string();
            break;
        }
      }

      if (local_ip.empty()) {
        throw std::runtime_error("No suitable IPv4 address found");
      }
    } catch (const std::exception& e) {
        std::cerr << "[Client][ERROR] Failed to retrieve local IP: " << e.what() << std::endl;
        return;
    }

    auto udp_info_packet = network::CreateUdpPortPacket<network::MyPacketType>(udp_port, local_ip);

    if (!udp_info_packet) {
        std::cerr << "[ERROR] Failed to create UDP port packet. Aborting send." << std::endl;
        return;
    }

    network_server_.SendTcp(std::move(*udp_info_packet));


    std::cout << "[Client][INFO] Sent UDP port (" << udp_port
              << ") and IP (" << local_ip << ") to server." << std::endl;
  });

  event_queue_.Subscribe(EventType::UpdatePlayers, [this](const network::Packet<network::MyPacketType>& packet) {
        const auto extracted_data =
            network::PacketFactory<network::MyPacketType>::ExtractDataArray<network::packets::UpdatePlayer>(packet);

    if (!extracted_data) {
        std::cerr << "[Client][ERROR] Failed to extract PlayerUpdate data from packet: invalid size." << std::endl;
        //client_.Shutdown();
        return;
    }

    const auto& update_players = *extracted_data;
    auto& positions = registry_.get_components<Position>();
    auto& client_player = registry_.get_components<ClientPlayer>();

    for (const auto& [player_id, new_x, new_y, score, health] : update_players) {
        if (const auto entity = game_state_.GetPlayer(player_id);
            entity == client::GameState::InvalidEntity) {
            std::cout << "[Client][INFO] Adding new player with ID: " << static_cast<int>(player_id) << std::endl;

            game_state_.AddPlayer(player_id, new_x, new_y, score, health);

            std::cout << "[Client][INFO] Added Player " << static_cast<int>(player_id)
                      << " at position (" << new_x << ", " << new_y << ")" << std::endl;
        } else if (entity < positions.size() && positions[entity].has_value() && client_player[entity].has_value()) {
            auto& [x, y] = *positions[entity];
            auto& [_, player_score, player_health] = *client_player[entity];
            x = new_x;
            y = new_y;

            player_score = score;
            player_health = health;
        } else {
            std::cerr << "[Client][WARNING] Position component not found for Player ID: "
                      << static_cast<int>(player_id) << std::endl;
        }
    }
  });

  event_queue_.Subscribe(EventType::UpdateProjectiles, [this](const network::Packet<network::MyPacketType>& packet) {
        const auto extracted_data =
            network::PacketFactory<network::MyPacketType>::ExtractDataArray<network::packets::UpdateProjectile>(packet);

    if (!extracted_data) {
        std::cerr << "[Client][ERROR] Failed to extract ProjectileUpdate data from packet: invalid size." << std::endl;
        //client_.Shutdown();
        return;
    }

    const auto& update_projectiles = *extracted_data;
    auto& positions = registry_.get_components<Position>();

    for (const auto& [projectile_id, owner_id, new_x, new_y] : update_projectiles) {
        if (const auto entity = game_state_.GetProjectileEntity(projectile_id);
            entity == client::GameState::InvalidEntity) {

          game_state_.AddProjectile(projectile_id, owner_id, new_x, new_y);

        } else if (entity < positions.size() && positions[entity].has_value()) {
            auto& [x, y] = *positions[entity];
            x = new_x;
            y = new_y;

        } else {
            std::cerr << "[Client][WARNING] Position component not found for Projectile ID: "
                      << static_cast<int>(projectile_id) << std::endl;
        }
    }
  });

  event_queue_.Subscribe(EventType::UpdateEnemies, [this](const network::Packet<network::MyPacketType>& packet) {
        const auto extracted_data =
            network::PacketFactory<network::MyPacketType>::ExtractDataArray<network::packets::UpdateEnemy>(packet);

    if (!extracted_data) {
        std::cerr << "[Client][ERROR] Failed to extract EnemyUpdate data from packet: invalid size." << std::endl;
        //client_.Shutdown();
        return;
    }

    const auto& update_enemies = *extracted_data;
    auto& positions = registry_.get_components<Position>();
    auto& enemies = registry_.get_components<Enemy>();

    for (const auto& [enemy_id, new_x, new_y] : update_enemies) {
        if (const auto entity = game_state_.GetEnemy(enemy_id);
            entity == client::GameState::InvalidEntity) {
            game_state_.AddEnemy(enemy_id, new_x, new_y);

        } else if (entity < positions.size() && positions[entity].has_value() && enemies[entity].has_value()) {
            auto& [x, y] = *positions[entity];
            x = new_x;
            y = new_y;

        } else {
            std::cerr << "[Client][WARNING] Position component not found for Enemy ID: "
                      << static_cast<int>(enemy_id) << std::endl;
        }
    }
  });

  ConnectToGameServer();
}

void GameScene::Exit() {
  std::cout << "[GameScene] Exit()" << std::endl;
}

void GameScene::Update(float /*delta_time*/) {
  // Update game logic here if needed
}

void GameScene::RenderMapBorders() const {
  constexpr glm::vec2 left_bar_position = {-200.0f, -2100.0f};
  constexpr glm::vec2 left_bar_size = {400.0f, 2200.0f};
  renderer_.DrawVisibleVerticalBar(left_bar_position, left_bar_size);

  constexpr glm::vec2 right_bar_position = {1800.0f, -2100.0f};
  constexpr glm::vec2 right_bar_size = {400.0f, 2200.0f};
  renderer_.DrawVisibleVerticalBar(right_bar_position, right_bar_size);

  constexpr glm::vec2 bottom_bar_position = {-100.0f, -2200.0f};
  constexpr glm::vec2 bottom_bar_size = {2200.0f, 400.0f};
  renderer_.DrawVisibleHorizontalBar(bottom_bar_position, bottom_bar_size);

  constexpr glm::vec2 top_bar_position = {-100.0f, -200.0f};
  constexpr glm::vec2 top_bar_size = {2200.0f, 400.0f};
  renderer_.DrawVisibleHorizontalBar(top_bar_position, top_bar_size);
}

void GameScene::RenderEntities() {
  auto& positions = registry_.get_components<Position>();
  auto& players = registry_.get_components<ClientPlayer>();
  auto& enemies = registry_.get_components<Enemy>();
  auto& projectiles = registry_.get_components<Projectile>();

  for (size_t i = 0; i < positions.size(); ++i) {
    if (!positions[i].has_value()) {
      continue;
    }

    const auto& [x, y] = *positions[i];
    glm::vec2 screen_position = {x, -y};

    if (players[i].has_value()) {
      renderer_.DrawStarguy(screen_position, {120.0f, 120.0f});
    } else if (enemies[i].has_value()) {
      renderer_.DrawEnemy(screen_position, {30.0f, 30.0f});
    } else if (projectiles[i].has_value()) {
      renderer_.DrawProjectile(screen_position, {120.0f, 120.0f});
    }
  }
}

void GameScene::Render() {
  const auto [x1, y1] = game_state_.GetLocalPlayerPosition();

  renderer_.UpdateCamera({x1, y1});

  RenderMapBorders();

  RenderEntities();

  if (connection_status_text_) {
  //  connection_status_text_->Render(renderer_.GetSDLRenderer());
  }
}

void GameScene::HandleInput(const SDL_Event& event) {
  input_manager_.HandleEvent(event);
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
  //  connection_status_text_->SetContent("Connected to game server!");
  //  connection_status_text_->SetColor(SDL_Color{0, 255, 0, 255});
  //  std::cout << "[GameScene] Successfully connected to the game server." << std::endl;
  } else {
    connection_status_text_->SetContent("Failed to connect to game server.");
    connection_status_text_->SetColor(SDL_Color{255, 0, 0, 255});
    std::cerr << "[GameScene][ERROR] Connection to game server failed." << std::endl;
  }
}

void GameScene::HandlePlayerInput(InputManager::PlayerInput&& input)
{
  auto input_packet = network::CreatePlayerInputPacket<network::MyPacketType>(
      client_id_, input.actions, input.dir_x, input.dir_y);

  if (input_packet) {
    network_server_.SendUdp(std::move(*input_packet));
  } else {
    std::cerr << "[GameScene][ERROR] Failed to create player input packet." << std::endl;
  }
}

}  // namespace rtype
