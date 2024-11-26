#ifndef GAME_SERVER_HPP_
#define GAME_SERVER_HPP_

#include <cstdint>
#include <network/network_server.hpp>
#include <thread>

#include "logic.hpp"
#include "message_dispatcher.hpp"

namespace game {
  class GameServer final {
    public:
      explicit GameServer(const uint16_t port) : network_server_(port), running_(false) {};
      ~GameServer() { stop(); }

      bool start() {
        if (!network_server_.start()) {
          std::cerr << "[GameServer][ERROR] Failed to start network server.\n";
          return false;
        }

        running_ = true;
        game_thread_ = std::thread([this]() { run(); });
        std::cout << "[GameServer][INFO] Game server started successfully.\n";
        return true;
      }

    void stop() {
      if (!running_) return;

      running_ = false;
      network_server_.stop();

      if (game_thread_.joinable()) {
        game_thread_.join();
      }

      std::cout << "[GameServer][INFO] Game server stopped.\n";
    }

    private:
    void run() {
      constexpr auto tick_duration = std::chrono::milliseconds(16);
      thread_local auto next_tick_time = std::chrono::steady_clock::now();

      while (running_) {
        auto current_time = std::chrono::steady_clock::now();
        auto delta_time = std::chrono::duration_cast<std::chrono::milliseconds>(
          current_time - next_tick_time + tick_duration
        );

        process_packets(50, std::chrono::milliseconds(10));

        // Update game logic
        // logic_.update(delta_time);

        next_tick_time += tick_duration;
        if (auto sleep_time = next_tick_time - std::chrono::steady_clock::now();
            sleep_time > std::chrono::milliseconds(0)) {
          std::this_thread::sleep_for(sleep_time);
        } else {
          std::cerr << "[GameServer] Tick overrun by "
                    << -sleep_time.count() << " ms\n";
          next_tick_time = std::chrono::steady_clock::now();
        }
      }
    }

    void process_packets(const int max_packets,
                         const std::chrono::milliseconds max_time) {
      const auto start_time = std::chrono::steady_clock::now();
      int processed = 0;

      while (processed < max_packets) {
        auto packet_opt = network_server_.pop_message();
        if (!packet_opt) break;

        // Time check
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            current_time - start_time);

        if (elapsed >= max_time) break;

        MessageDispatcher::dispatch(std::move(packet_opt.value()));
        ++processed;
      }
    }

    network::NetworkServer network_server_;
    Logic logic_;
    bool running_;
    std::thread game_thread_;
  };
}

#endif // GAME_SERVER_HPP_
