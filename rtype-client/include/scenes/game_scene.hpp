#ifndef GAME_SCENE_HPP_
#define GAME_SCENE_HPP_

#include <SDL2/SDL_ttf.h>

#include <memory>
#include <string>
#include <vector>

#include "core/game_state.hpp"
#include "core/input_manager.hpp"
#include "core/my_packet_types.hpp"
#include "core/service_locator.hpp"
#include "network/network_client.hpp"
#include "scene.hpp"
#include "scene_manager.hpp"
#include "ui/text.hpp"

namespace rtype {

class GameScene final : public Scene {
public:
  explicit GameScene(std::string  ip_address, const std::vector<int>& ports);
  ~GameScene() override = default;

  void Enter() override;
  void Exit() override;
  void Update(float delta_time) override;
  void Render() override;
  void HandleInput(const SDL_Event& event) override;

  [[nodiscard]] uint8_t GetClientId() const { return client_id_; }
  void SetClientId(const uint8_t id) { client_id_ = id; }

private:
  void InitializeUI();
  void ConnectToGameServer();
  void HandlePlayerInput(InputManager::PlayerInput&& input);

  void RenderMapBorders() const;
  void RenderEntities();

  std::string ip_address_;
  std::vector<int> ports_;

  bool is_connected_;
  uint8_t client_id_{0};

  Renderer& renderer_;
  SceneManager& scene_manager_;
  EventQueue<network::Packet<network::MyPacketType>>& event_queue_;

  network::NetworkClient<network::MyPacketType>& network_server_;
  network::NetworkClient<network::MyPacketType>& game_server_;

  ScreenManager screen_manager_{};
  InputManager input_manager_;

  Registry registry_{};
  client::GameState game_state_;
};

}  // namespace rtype

#endif  // GAME_SCENE_HPP_
