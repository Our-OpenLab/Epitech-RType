#ifndef RTYPE_CLIENT_CORE_MAIN_SERVER_HPP_
#define RTYPE_CLIENT_CORE_MAIN_SERVER_HPP_

#include <chrono>
#include <cstdint>
#include <iostream>
#include <memory>
#include <network/network_client.hpp>
#include <string>
#include <thread>

#include "core/renderer.hpp"
#include "event_queue.hpp"
#include "message_dispatcher.hpp"
#include "scenes/login_scene.hpp"
#include "scenes/scene_manager.hpp"
#include "service_locator.hpp"

namespace rtype {

/**
 * @brief Orchestrates the central client operations for RType.
 *
 * This client handles:
 * - Establishing TCP/UDP connections to the server.
 * - Sending and receiving network packets.
 * - Managing a centralized EventQueue for asynchronous events.
 *
 * It interacts with `GameState` to update local game data and communicates
 * with the server for synchronization.
 *
 * @tparam PacketType The type of packet to process, typically representing
 *         the various message types exchanged in the game.
 */
template <typename PacketType>
class MainServer {
  using PacketEventQueue = EventQueue<network::Packet<network::MyPacketType>>;
public:
  /**
  * @brief Constructs a MainServer instance.
  *
  * Initializes the message dispatcher and event queue resources. The
  * message dispatcher is tied to the event queue for publishing events
  * when unrecognized packets arrive or specific packet types are handled.
  */
 explicit MainServer(const std::string& local_ip)
  : network_server_(std::make_shared<network::NetworkClient<PacketType>>()),
  game_server_(std::make_shared<network::NetworkClient<PacketType>>()),
  active_server_(network_server_),
  event_queue_(std::make_shared<PacketEventQueue>()),
  message_dispatcher_(*event_queue_),
  renderer_(std::make_shared<Renderer>(1280, 960, "RType Client")),
  scene_manager_(std::make_shared<SceneManager>()) {
    ServiceLocator::Provide("renderer", renderer_);
    ServiceLocator::Provide("event_queue", event_queue_);
    ServiceLocator::Provide("network_server", network_server_);
    ServiceLocator::Provide("game_server", game_server_);
    ServiceLocator::Provide("scene_manager", scene_manager_);
    ServiceLocator::Provide("main_server", this);
    ServiceLocator::Provide("local_ip", std::make_shared<std::string>(local_ip));

    event_queue_->Subscribe(EventType::Pong, [this](const network::Packet<network::MyPacketType>& packet) {
      if (packet.body.size() != sizeof(network::packets::PingPacket)) {
          std::cerr << "[Client][ERROR] Invalid PingPacket size received." << std::endl;
          return;
      }

      const auto* ping_packet = reinterpret_cast<const network::packets::PingPacket*>(packet.body.data());

      const uint32_t timestamp = ping_packet->timestamp;

      // std::cout << "[Client][INFO] Received ping with timestamp: " << timestamp << " ms" << std::endl;
    });
  }


 /**
  * @brief Destructor for ClientMainServer.
  *
  * Ensures the client stops gracefully by cleaning up resources
  * and joining the client thread if it's running.
  */
  ~MainServer();

  /**
   * @brief Starts the ClientMainServer.
   *
   * This includes:
   * - Connecting to the server.
   * - Starting the client loop in a dedicated thread.
   *
   * @return True if the client started successfully, false otherwise.
   */
  bool Start(const std::string& host, const std::string& service, uint16_t udp_port);

  /**
   * @brief Stops the ClientMainServer.
   *
   * Safely shuts down the client, terminates the client loop,
   * and joins the client thread.
   */
  void Stop();

  /**
   * @brief Switches the active server connection to the network server.
   */
  void SwitchToNetworkServer() {
    active_server_ = network_server_;
  }

  /**
   * @brief Switches the active server connection to the game server.
   */
  void SwitchToGameServer() {
    active_server_ = game_server_;
  }

  [[nodiscard]] std::shared_ptr<network::NetworkClient<PacketType>> GetActiveServer() const {
    return active_server_;
  }

  /**
  * @brief Interprets or routes the input commands read from stdin.
  *
  * Here you can parse commands like "quit", "ping", etc.
  * This is just a placeholder; adapt to your needs.
  *
  * @param cmd The command string entered by the user.
  */
  void HandleCommand(const std::string& cmd);

  std::vector<std::string> ParseCommandArgs(const std::string& cmd);

  void RegisterUser(const std::string& username, const std::string& password);

  void LoginUser(const std::string& username, const std::string& password);

  void SendMessageToPlayer(int recipient_id, const std::string& message_content);

  void CreateLobby(const std::string& name, const std::optional<std::string>& password);

  void JoinLobby(int lobby_id, const std::optional<std::string>& password);

  void SetReadyStatus(bool is_ready);


  /**
   * @brief Indicates whether the client is currently running.
   *
   * @return True if the client is running, false otherwise.
   */
  [[nodiscard]] bool IsRunning() const { return is_running_; }

private:
  /**
   * @brief Runs the main client loop.
   *
   * The loop performs periodic tasks, such as:
   * - Sending ping messages to the server.
   * - Processing incoming packets.
   * - Managing events from the EventQueue.
   */
  void Run();

  /**
   * @brief Processes incoming packets from the network.
   *
   * This fetches packets from the network server's queue and delegates
   * them for handling.
   *
   * @param max_packets Maximum number of packets to process in one iteration.
   * @param max_time Maximum duration to spend processing packets.
   */
  void ProcessPackets(int max_packets, std::chrono::milliseconds max_time);

  void SendPing() const;

  //GameState game_state_;  ///< Manages the local game state.
  std::shared_ptr<network::NetworkClient<PacketType>> network_server_;   ///< The network server connection.
  std::shared_ptr<network::NetworkClient<PacketType>> game_server_;   ///< The game server connection.
  std::shared_ptr<network::NetworkClient<PacketType>> active_server_;  ///< The active server connection.
  std::shared_ptr<PacketEventQueue> event_queue_;  ///< Centralized queue for managing asynchronous events.
  network::MessageDispatcher<PacketType> message_dispatcher_;  ///< Routes incoming packets to their appropriate handlers.
  std::shared_ptr<Renderer> renderer_;  ///< Manages the rendering context.
  std::shared_ptr<SceneManager> scene_manager_; ///< Manages the active scene and scene transitions.

  bool is_running_{false};  ///< Indicates whether the client is currently running.
};

}

#include "main_server.tpp"

#endif
