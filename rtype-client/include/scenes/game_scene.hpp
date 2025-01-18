#ifndef GAME_SCENE_HPP_
#define GAME_SCENE_HPP_

#include <memory>
#include <string>
#include <vector>
#include <SDL2/SDL_ttf.h>

#include "scene.hpp"
#include "scene_manager.hpp"
#include "ui/text.hpp"
#include "core/my_packet_types.hpp"
#include "core/service_locator.hpp"
#include "network/network_client.hpp"

namespace rtype {

class GameScene final : public Scene {
public:
  explicit GameScene(const std::string& ip_address, const std::vector<int>& ports);
  ~GameScene() override = default;

  void Enter() override;
  void Exit() override;
  void Update(float delta_time) override;
  void Render() override;
  void HandleInput(const SDL_Event& event) override;

private:
  void InitializeUI();
  void ConnectToGameServer();

  std::string ip_address_;
  std::vector<int> ports_;
  bool is_connected_;

  TTF_Font* font_{nullptr};
  std::unique_ptr<Text> connection_status_text_;
  Renderer& renderer_;
};

}  // namespace rtype

#endif  // GAME_SCENE_HPP_
