#ifndef REGISTER_SCENE_HPP_
#define REGISTER_SCENE_HPP_

#include <SDL2/SDL_ttf.h>

#include <iostream>
#include <memory>
#include <string>

#include "core/my_packet_types.hpp"
#include "core/packet_factory.hpp"
#include "core/resource_manager.hpp"
#include "core/service_locator.hpp"
#include "scene.hpp"
#include "scene_manager.hpp"
#include "ui/button.hpp"
#include "ui/text.hpp"
#include "ui/text_box.hpp"
#include "ui/text_link.hpp"

namespace rtype {

class LoginScene;

class RegisterScene final : public Scene {
public:
  explicit RegisterScene();
  ~RegisterScene() override = default;

  void Enter() override;
  void Exit() override;
  void Update(float delta_time) override;
  void Render() override;
  void HandleInput(const SDL_Event& e) override;

private:
  void InitializeResources();
  void CreateUIElements();
  void OnRegisterButtonClicked();
  void HandleRegisterResponse(const network::packets::RegisterResponsePacket& packet);

  TTF_Font* font_{nullptr};
  std::unique_ptr<Text> title_text_;
  std::unique_ptr<Text> username_label_;
  std::unique_ptr<TextBox> username_box_;
  std::unique_ptr<Text> password_label_;
  std::unique_ptr<TextBox> password_box_;
  std::unique_ptr<Text> confirm_password_label_;
  std::unique_ptr<TextBox> confirm_password_box_;
  std::unique_ptr<TextButton> register_button_;
  std::unique_ptr<Text> status_text_;
  std::unique_ptr<TextLink> login_link_;

  bool is_waiting_;

  Renderer& renderer_;
  SceneManager& scene_manager_;
  EventQueue<network::Packet<network::MyPacketType>>& event_queue_;
  network::NetworkClient<network::MyPacketType>& network_server_;
};

}  // namespace rtype

#endif  // REGISTER_SCENE_HPP_
