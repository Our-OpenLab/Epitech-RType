#ifndef RTYPE_CLIENT_CORE_MAIN_SERVER_HPP_
#define RTYPE_CLIENT_CORE_MAIN_SERVER_HPP_

#include <chrono>
#include <cstdint>
#include <iostream>
#include <memory>
#include <network/network_client.hpp>
#include <string>
#include <thread>

#include "event_queue.hpp"
#include "message_dispatcher.hpp"

#include "core/renderer.hpp"
#include "scenes/login_scene.hpp"
#include "scenes/scene_manager.hpp"

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
public:
  /**
   * @brief Constructs a MainServer instance.
   *
   * Initializes the message dispatcher and event queue resources. The
   * message dispatcher is tied to the event queue for publishing events
   * when unrecognized packets arrive or specific packet types are handled.
   */
  MainServer()
  : network_server_(std::make_shared<network::NetworkClient<PacketType>>())
  , message_dispatcher_(event_queue_)
  , renderer_("RType Client", 1280, 720)
  , sceneManager_(
           // we can build a unique_ptr<Scene> inline with a second lambda or
           // something
           [this]() -> SceneManager {
             auto loginScene = std::make_unique<LoginScene>(
                 renderer_, font_);
             return SceneManager(std::move(loginScene));
           }()) // calls the lambda
  // here's the tricky part: we must create the scene only after the font is created
  , font_([] {
         // Lambda that calls TTF_Init and TTF_OpenFont, returning TTF_Font*
         if (TTF_Init() != 0) {
           std::cerr << "TTF_Init Error: " << TTF_GetError() << std::endl;
           // handle or throw
         }
         TTF_Font* f = TTF_OpenFont("assets/fonts/Roboto-Regular.ttf", 24);
         if (!f) {
           std::cerr << "OpenFont Error: " << TTF_GetError() << std::endl;
           // handle or throw
         }
         return f;
       }())
{
    // Now the compiler sees sceneManager_ is built in the initializer.
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
  std::shared_ptr<network::NetworkClient<PacketType>> network_server_{};  ///< Handles all network communication.
  network::MessageDispatcher<PacketType> message_dispatcher_;  ///< Routes incoming packets to their appropriate handlers.
  EventQueue event_queue_{};  ///< Centralized queue for managing asynchronous events.
  std::thread client_thread_;  ///< Thread for running the client loop.

  bool is_running_{false};  ///< Indicates whether the client is currently running.

  Renderer renderer_;
  SceneManager sceneManager_;
  TTF_Font* font_;
};

}

#include "main_server.tpp"

#endif
