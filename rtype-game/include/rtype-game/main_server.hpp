#ifndef MAIN_SERVER_HPP_
#define MAIN_SERVER_HPP_

#include <chrono>
#include <cstdint>
#include <thread>

#include "engine/game_engine.hpp"
#include "event_queue.hpp"
#include "game_network_server.hpp"
#include "engine/game_state.hpp"
#include "message_dispatcher.hpp"
#include "service_container.hpp"

namespace rtype {

/**
 * @brief Orchestrates the central server operations for RType.
 *
 * This server handles:
 * - Network communication via TCP/UDP.
 * - Processing and dispatching network packets.
 * - Maintaining and updating the game state.
 * - Running a main game loop in a separate thread.
 * - Managing a centralized EventQueue for asynchronous events.
 *
 * It interacts with `GameState` to manage player data and game sessions, and
 * uses `GameNetworkServer` for client communication.
 *
 * @tparam PacketType The type of packet to process, typically representing
 *         the various message types exchanged in the game.
 */
template <typename PacketType>
class MainServer {
public:
  /**
   * @brief Constructs the MainServer with specific network ports and services.
   *
   * This sets up the network server, initializes the game state, the centralized
   * EventQueue, and the ServiceContainer for managing services.
   *
   * @param tcp_port Port for TCP communication.
   * @param udp_port Port for UDP communication.
   */
  explicit MainServer(uint16_t tcp_port, uint16_t udp_port);

  /**
   * @brief Destructor for MainServer.
   *
   * Ensures the server stops gracefully by cleaning up resources
   * and joining the game thread if it's running.
   */
  ~MainServer();

  /**
   * @brief Starts the MainServer.
   *
   * This includes:
   * - Initializing the network server.
   * - Starting the game loop in a dedicated thread.
   *
   * @return True if the server started successfully, false otherwise.
   */
  bool Start();

  /**
   * @brief Stops the MainServer.
   *
   * Safely shuts down the network server, terminates the game loop,
   * and joins the server thread.
   */
  void Stop();

private:
  /**
   * @brief Runs the main game loop.
   *
   * The loop performs periodic tasks, such as:
   * - Processing network packets.
   * - Updating the game state.
   * - Sending updates to clients at regular intervals.
   * - Processing queued events.
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

  std::shared_ptr<network::GameNetworkServer<PacketType>> network_server_;  ///< Handles all network communication.
  network::MessageDispatcher<PacketType> message_dispatcher_; ///< Routes incoming packets to their appropriate handlers.
  EventQueue event_queue_;  ///< Centralized queue for managing asynchronous events.
  std::thread game_thread_; ///< Thread that runs the server's main game loop.
  GameEngine game_engine_{};  ///< Handles game logic and systems.
  GameState game_state_;  ///< Represents the game state.

  bool is_running_{false};  ///< Indicates whether the server is currently running.
};

}  // namespace rtype

#include "main_server.tpp"

#endif  // MAIN_SERVER_HPP_
