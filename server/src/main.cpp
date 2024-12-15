#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>

#include <server/core/game_server.hpp>

template <typename PacketType>
class ServerController {
public:
  explicit ServerController(game::GameServer<PacketType>& server)
      : server_(server) {}

  void Start() {
    std::signal(SIGINT, [](int) { stop_requested_ = true; });

    if (!server_.Start()) {
      std::cerr << "[Main][ERROR] Failed to start the game server.\n";
      return;
    }

    while (keep_running_) {
      if (stop_requested_) {
        stop();
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  void stop() {
    if (!keep_running_) return;

    keep_running_ = false;
    server_.Stop();
    std::cout << "[Main] Server shutdown completed.\n";
  }

private:
  game::GameServer<PacketType>& server_;
  std::atomic<bool> keep_running_{true};
  static inline std::atomic<bool> stop_requested_{false};
};

int main()
{
  game::GameServer<network::MyPacketType> game_server(4242, 4243);

  ServerController controller(game_server);

  controller.Start();

  return 0;
}
