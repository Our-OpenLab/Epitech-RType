#include <SDL.h>
#include <iostream>

#include "client/core/client.hpp"
#include "client/core/message_dispatcher.hpp"

Client::Client(const std::string& host, const std::string& tcp_port, const uint16_t udp_port)
  : renderer_(1280, 960, "R-Type"),
      network_client_(host, tcp_port, udp_port),
      message_dispatcher_(std::make_unique<network::MessageDispatcher>(*this)),
      input_manager_([this](InputManager::PlayerInput&& input) {
        const network::PlayerInput network_input{
          .player_id = client_id_,
          .actions = input.actions,
          .dir_x = input.dir_x,
          .dir_y = input.dir_y,
          .timestamp = input.timestamp
        };

        auto input_packet = network::PacketFactory<network::MyPacketType>::CreatePacket(
             network::MyPacketType::kPlayerInput, network_input);

        network_client_.SendUdp(std::move(input_packet));
      }, screen_manager_),
      game_state_(game_engine_.GetRegistry()) {
//  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
//    throw std::runtime_error("SDL_Init failed: " + std::string(SDL_GetError()));
//  }

  screen_manager_.InitializeScreenDimensions();

  game_engine_.InitializeSystems();

  std::cout << "[Client][INFO] Initialization completed successfully." << std::endl;
}

Client::~Client() {
  Shutdown();
}

void Client::Run() {
    uint64_t tick_counter = 0;
    uint64_t last_ping_tick = 0;
    auto next_tick_time = std::chrono::steady_clock::now();

    while (is_running_) {
        const auto tick_start_time = std::chrono::steady_clock::now();
        //const float delta_time =
        //    std::chrono::duration<float>(tick_start_time - next_tick_time).count();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                is_running_ = false;
            }
            input_manager_.HandleEvent(event, tick_start_time);
        }

        ProcessPackets(kMaxPacketsPerTick, kMaxPacketProcessingTime);

      if (tick_counter - last_ping_tick >= kPingFrequencyTicks) {
        const auto timestamp = static_cast<uint32_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                tick_start_time.time_since_epoch())
                .count());
        SendPing(timestamp);
        last_ping_tick = tick_counter;
      }

      // game_engine_.Update(delta_time, render_time);

      const auto [x1, y1] = game_state_.GetLocalPlayerPosition();

      renderer_.UpdateCamera({x1, y1});
      renderer_.Clear();
      renderer_.DrawGame(game_state_);
      renderer_.Present();

      ++tick_counter;
      next_tick_time += kTickDuration;

      if (const auto sleep_time = next_tick_time - std::chrono::steady_clock::now();
          sleep_time > std::chrono::milliseconds(0)) {
        std::this_thread::sleep_for(sleep_time);
      } else {
        std::cerr << "[Client][WARNING] Tick overrun by "
                  << -sleep_time.count() << " ms\n";
        next_tick_time = std::chrono::steady_clock::now();
      }
    }
}

void Client::SendPing(const uint32_t timestamp) const {
  auto ping_packet = network::PacketFactory<network::MyPacketType>::CreatePacket(
      network::MyPacketType::kPing, timestamp);

  network_client_.SendUdp(std::move(ping_packet));

  std::cout << "[Client][INFO] Ping sent with timestamp: " << timestamp << " ms\n";
}


void Client::ProcessPackets(const int max_packets,
                             const std::chrono::milliseconds max_time) {
  {
    const auto start_time = std::chrono::steady_clock::now();
    int processed = 0;

    while (processed < max_packets) {
      auto packet_opt = network_client_.PopMessage();
      if (!packet_opt) break;

      auto current_time = std::chrono::steady_clock::now();
      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
          current_time - start_time);

      if (elapsed >= max_time) break;

      message_dispatcher_->Dispatch(std::move(packet_opt.value()));
      ++processed;
    }
  }
}


void Client::Shutdown() {
  std::cout << "[Client][INFO] Shutting down client..." << std::endl;

  renderer_.Shutdown();
  SDL_Quit();

  std::cout << "[Client][INFO] Shutdown completed." << std::endl;
}