#ifndef MAIN_MENU_SCENE_HPP_
#define MAIN_MENU_SCENE_HPP_

#include <memory>
#include <network/network_client.hpp>
#include <string>

#include "core/my_packet_types.hpp"
#include "core/packet_factory.hpp"
#include "scene.hpp"
#include "scene_manager.hpp"
#include "ui/button.hpp"
#include "ui/chat_overlay.hpp"
#include "ui/text.hpp"

namespace rtype {

class MainMenuScene final : public Scene {
public:
  MainMenuScene();
  ~MainMenuScene() override = default;

  void Enter() override;
  void Exit() override;
  void Update(float delta_time) override;
  void Render() override;
  void HandleInput(const SDL_Event& event) override;

private:
  void InitializeUI();
  void OnPlayButtonClicked();
  void OnSettingsButtonClicked();
  void OnExitButtonClicked();

  TTF_Font* font_{nullptr};
  std::unique_ptr<Text> title_text_;
  std::unique_ptr<TextButton> play_button_;
  std::unique_ptr<TextButton> settings_button_;
  std::unique_ptr<TextButton> exit_button_;

  Renderer& renderer_;
  SceneManager& scene_manager_;
 // EventQueue<network::Packet<network::MyPacketType>>& event_queue_;
 // network::NetworkClient<network::MyPacketType>& network_server_;
  std::shared_ptr<ChatOverlay> chat_overlay_;
};

}  // namespace rtype

#endif  // MAIN_MENU_SCENE_HPP_
