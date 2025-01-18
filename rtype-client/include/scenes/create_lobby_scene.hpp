#ifndef CREATE_LOBBY_SCENE_HPP_
#define CREATE_LOBBY_SCENE_HPP_

#include <memory>
#include <string>
#include <SDL2/SDL_ttf.h>

#include "scene.hpp"
#include "scene_manager.hpp"
#include "ui/button.hpp"
#include "ui/text.hpp"
#include "ui/text_box.hpp"
#include "core/my_packet_types.hpp"
#include "core/packet_factory.hpp"
#include "core/service_locator.hpp"
#include "network/network_client.hpp"

namespace rtype {

class CreateLobbyScene final : public Scene {
public:
  explicit CreateLobbyScene();
  ~CreateLobbyScene() override = default;

  void Enter() override;
  void Exit() override;
  void Update(float delta_time) override;
  void Render() override;
  void HandleInput(const SDL_Event& event) override;

private:
  void InitializeUI();
  void OnCreateButtonClicked();
  void OnCancelButtonClicked();
  void HandleCreateLobbyResponse(const network::packets::CreateLobbyResponsePacket& packet);

  TTF_Font* font_{nullptr};
  std::unique_ptr<Text> title_text_;
  std::unique_ptr<Text> name_label_;
  std::unique_ptr<TextBox> name_box_;
  std::unique_ptr<Text> password_label_;
  std::unique_ptr<TextBox> password_box_;
  std::unique_ptr<TextButton> create_button_;
  std::unique_ptr<TextButton> cancel_button_;
  std::unique_ptr<Text> status_text_;

  Renderer& renderer_;
  SceneManager& scene_manager_;
  EventQueue<network::Packet<network::MyPacketType>>& event_queue_;
  network::NetworkClient<network::MyPacketType>& network_server_;
};

}  // namespace rtype

#endif  // CREATE_LOBBY_SCENE_HPP_
