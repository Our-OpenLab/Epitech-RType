#ifndef SERVER_CORE_GAME_SERVER_H_
#define SERVER_CORE_GAME_SERVER_H_

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <thread>

#include "shared/network_messages.hpp"
#include "server/core/custom_network_server.hpp"
#include "server/core/event_queue.hpp"
#include "server/core/message_dispatcher.hpp"
#include "server/engine/game_engine.hpp"
#include "server/engine/game_state.hpp"

namespace game {

/**
 * @brief Main class for managing the game server.
 *
 * Handles networking, game logic, and synchronization of game state with clients.
 *
 * @tparam PacketType Enum class representing the packet types used in the game.
 */
template <typename PacketType>
class GameServer {
 public:
  /**
   * @brief Constructs the game server.
   *
   * @param tcp_port Port for TCP communication.
   * @param udp_port Port for UDP communication.
   */
  explicit GameServer(uint16_t tcp_port, uint16_t udp_port);

  /**
   * @brief Destructor for the game server.
   *
   * Ensures proper shutdown and cleanup of resources.
   */
  ~GameServer();

  /**
   * @brief Starts the game server.
   *
   * Initializes the networking and game engine.
   * @return True if the server starts successfully, false otherwise.
   */
  bool Start();

  /**
   * @brief Stops the game server.
   *
   * Terminates networking, game logic, and worker threads.
   */
  void Stop();

 private:
  // Constants
  static constexpr std::chrono::milliseconds kTickDuration{8};  ///< Tick duration (~62.5 ticks/sec).
  static constexpr int kMaxPacketsPerTick{50};                   ///< Maximum packets to process per tick.
  static constexpr std::chrono::milliseconds kMaxPacketProcessingTime{10};  ///< Maximum time to process packets per tick.
  static constexpr std::size_t kSafeUdpPayloadSize{512};        ///< Safe UDP payload size (bytes).
  static constexpr uint32_t kUpdateFrequencyTicks{1};           ///< Frequency of regular updates.
  static constexpr uint32_t kFullUpdateFrequencyTicks{4};       ///< Frequency of full state updates.

  /**
   * @brief Computes the maximum number of updates per UDP packet.
   *
   * This is dynamically calculated based on the size of the update structure.
   * @tparam UpdateType The type of update (e.g., UpdatePlayer).
   * @return Maximum number of updates that fit within the UDP payload size.
   */
  template <typename UpdateType>
  static constexpr std::size_t GetMaxUpdatesPerPacket() {
    return kSafeUdpPayloadSize / sizeof(UpdateType);
  }

  // Core Methods
  /**
   * @brief Main loop for the game server.
   *
   * Handles game logic, network synchronization, and periodic updates.
   */
  void Run();

  /**
   * @brief Processes incoming network packets.
   *
   * @param max_packets Maximum number of packets to process.
   * @param max_time Maximum time allocated for packet processing.
   */
  void ProcessPackets(int max_packets, std::chrono::milliseconds max_time);

  /**
   * @brief Sends regular updates to clients.
   *
   */
  void SendUpdatesToClients();

  /**
   * @brief Sends full state updates to clients.
   *
   * Sends the entire game state (players, enemies, projectiles) to all clients.
   */
  void SendFullStateUpdates();

  // Specific Entity Update Methods
  /**
   * @brief Sends player updates to clients.
   *
   * @param force_update If true, updates all players regardless of dirty flags.
   */
  void SendPlayerUpdates(bool force_update);

  /**
   * @brief Sends enemy updates to clients.
   *
   * @param force_update If true, updates all enemies regardless of dirty flags.
   */
  void SendEnemyUpdates(bool force_update);

  /**
   * @brief Sends projectile updates to clients.
   *
   * @param force_update If true, updates all projectiles regardless of dirty flags.
   */
  void SendProjectileUpdates(bool force_update);

  /**
   * @brief Sends a batch of updates in a single packet.
   *
   * Optimized for performance with fixed-size storage.
   *
   * @tparam T The type of updates (e.g., Player, Enemy, or Projectile updates).
   * @param updates A fixed-size container of updates.
   * @param count The number of valid updates in the container.
   * @param type The packet type representing the update.
   */
  template <typename T, std::size_t MaxUpdates>
  void SendUpdatePacket(const std::array<T, MaxUpdates>& updates, size_t count, PacketType type);

  // Members
  network::CustomNetworkServer<PacketType> network_server_;  ///< Handles network communication.
  GameEngine game_engine_;                                   ///< Handles game logic and systems.
  GameState game_state_;                                     ///< Represents the game state.
  EventQueue event_queue_;                                   ///< Queue for handling game events.
  std::thread game_thread_;                                  ///< Thread for running the game loop.
  bool running_ = false;                                     ///< Indicates whether the server is running.
};

}  // namespace game

#include "server/core/game_server.tpp"

#endif  // SERVER_CORE_GAME_SERVER_H_
