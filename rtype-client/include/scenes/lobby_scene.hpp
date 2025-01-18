#ifndef LOBBY_SCENE_HPP_
#define LOBBY_SCENE_HPP_

#include <memory>
#include <network/network_client.hpp>
#include <string>
#include <vector>
#include <unordered_map>

#include "core/resource_manager.hpp"
#include "core/service_locator.hpp"
#include "scene.hpp"
#include "scene_manager.hpp"
#include "ui/button.hpp"
#include "ui/text.hpp"

namespace network {
enum class MyPacketType : uint32_t;
}

namespace rtype {

class LobbyScene final : public Scene {
public:
  explicit LobbyScene(int lobby_id);
  ~LobbyScene() override = default;

  void Enter() override;
  void Exit() override;
  void Update(float delta_time) override;
  void Render() override;
  void HandleInput(const SDL_Event& event) override;

private:
  void InitializeUI();
  void OnReadyButtonClicked();
  void OnLeaveLobbyButtonClicked();
  void HandleGetLobbyPlayersResponse(const network::Packet<network::MyPacketType>& packet);
  void HandlePlayerReadyResponse(const network::Packet<network::MyPacketType>& packet);
  void HandleLeaveLobbyResponse(const network::Packet<network::MyPacketType>& packet);
  void HandlePlayerJoinedLobby(const network::Packet<network::MyPacketType>& packet);
  void HandlePlayerLeftLobby(const network::Packet<network::MyPacketType>& packet);
  void HandlePlayerReady(const network::Packet<network::MyPacketType>& packet);
  void HandleGameConnectionInfo(const network::Packet<network::MyPacketType>& packet);

  int lobby_id_;  // ID of the created lobby
  TTF_Font* font_{nullptr};
  std::unique_ptr<Text> title_text_;
  std::unique_ptr<Text> player_list_title_;
  std::vector<std::unique_ptr<Text>> player_list_;
  std::unique_ptr<TextButton> ready_button_;
  std::unique_ptr<TextButton> leave_lobby_button_;
  std::unordered_map<int, std::unique_ptr<Text>> player_map_;  ///< Maps player IDs to their Text objects.

  bool is_ready_ = false;  // État du joueur : prêt ou non
  Renderer& renderer_;
  SceneManager& scene_manager_;
  EventQueue<network::Packet<network::MyPacketType>>& event_queue_;
  network::NetworkClient<network::MyPacketType>& network_server_;
};

}  // namespace rtype

#endif  // LOBBY_SCENE_HPP_
