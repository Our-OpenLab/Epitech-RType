#ifndef LOBBY_PASSWORD_SCENE_HPP_
#define LOBBY_PASSWORD_SCENE_HPP_

#include <memory>
#include <string>
#include <SDL2/SDL_ttf.h>

#include "scene.hpp"
#include "scene_manager.hpp"
#include "ui/button.hpp"
#include "ui/text.hpp"
#include "ui/text_box.hpp"
#include "core/packet_factory.hpp"
#include "core/my_packet_types.hpp"
#include "core/service_locator.hpp"
#include "network/network_client.hpp"

namespace rtype {

class LobbyPasswordScene final : public Scene {
public:
  explicit LobbyPasswordScene(int lobby_id);
  ~LobbyPasswordScene() override = default;

  void Enter() override;
  void Exit() override;
  void Update(float delta_time) override;
  void Render() override;
  void HandleInput(const SDL_Event& event) override;

private:
  void InitializeUI();
  void OnSubmitButtonClicked();
  void OnCancelButtonClicked();
  void HandleJoinLobbyResponse(const network::packets::JoinLobbyResponsePacket& packet);

  int lobby_id_; ///< ID of the lobby being joined.
  TTF_Font* font_{nullptr};
  std::unique_ptr<Text> title_text_;
  std::unique_ptr<Text> password_label_;
  std::unique_ptr<TextBox> password_box_;
  std::unique_ptr<TextButton> submit_button_;
  std::unique_ptr<TextButton> cancel_button_;
  std::unique_ptr<Text> status_text_;

  Renderer& renderer_;
  SceneManager& scene_manager_;
  EventQueue<network::Packet<network::MyPacketType>>& event_queue_;
  network::NetworkClient<network::MyPacketType>& network_server_;
};

}  // namespace rtype

#endif  // LOBBY_PASSWORD_SCENE_HPP_
