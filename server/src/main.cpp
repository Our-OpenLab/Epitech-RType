#include <game/game_server.hpp>
#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>

class ServerController {
public:
  explicit ServerController(game::GameServer& server)
      : server_(server), keep_running_(true) {}

  void start() {
    std::signal(SIGINT, [](int) { stop_requested_ = true; });

    if (!server_.start()) {
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
    server_.stop();
    std::cout << "[Main] Server shutdown completed.\n";
  }

private:
  game::GameServer& server_;
  std::atomic<bool> keep_running_;
  static inline std::atomic<bool> stop_requested_ = false;
};

int main()
{
  game::GameServer game_server(4242);

  ServerController controller(game_server);

  controller.start();

  return 0;
}
