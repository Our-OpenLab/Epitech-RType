#include <SDL.h>
#include <iostream>

#include "client/core/client.hpp"
#include "client/core/message_dispatcher.hpp"

Client::Client(const std::string& host, const std::string& port)
  : renderer_(1920, 1080, "R-Type"),
      network_client_(host, port),
      input_manager_([this](InputManager::PlayerInput&& input) {
        const network::PlayerInput network_input{
          .player_id = client_id_,
          .actions = input.actions,
          .mouse_x = input.mouse_x,
          .mouse_y = input.mouse_y,
          .timestamp = input.timestamp
        };

        auto input_packet = network::PacketFactory<network::MyPacketType>::create_packet(
             network::MyPacketType::kPlayerInput, network_input);

        network_client_.send(std::move(input_packet));
      }, screen_manager_),
      game_state_(game_engine_.GetRegistry()) {
//  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
//    throw std::runtime_error("SDL_Init failed: " + std::string(SDL_GetError()));
//  }

//  screen_manager_.InitializeScreenDimensions();

  game_engine_.InitializeSystems();

  std::cout << "[Client][INFO] Initialization completed successfully." << std::endl;
}

Client::~Client() {
  Shutdown();
}
/*
void Client::Run() {
  uint64_t tick_counter = 0;
  uint64_t last_ping_tick = 0;
  auto next_tick_time = std::chrono::steady_clock::now();

  while (is_running_) {
    const auto current_time = std::chrono::steady_clock::now();
    const float delta_time =
        std::chrono::duration<float>(current_time - next_tick_time).count();

    const auto render_time = std::chrono::duration_cast<std::chrono::milliseconds>(
    current_time.time_since_epoch()) - kRenderDelay;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        is_running_ = false;
      }

      input_manager_.HandleEvent(event, current_time);
    }

    ProcessPackets(kMaxPacketsPerTick, kMaxPacketProcessingTime);

    if (tick_counter - last_ping_tick >= kPingFrequencyTicks) {
      SendPing(tick_counter);
      last_ping_tick = tick_counter;
    }

    game_engine_.Update(delta_time, render_time);
    //game_engine_.Update(delta_time);

    renderer_.Clear();
    renderer_.DrawGame(game_state_);
    renderer_.Present();

    ++tick_counter;
    next_tick_time += kTickDuration;

    if (const auto sleep_time =
            next_tick_time - std::chrono::steady_clock::now();
        sleep_time > std::chrono::milliseconds(0)) {
      std::this_thread::sleep_for(sleep_time);
        } else {
          std::cerr << "[Client] Tick overrun by "
                    << -sleep_time.count() << " ms\n";
          next_tick_time = std::chrono::steady_clock::now();
        }
  }
}
*/

void Client::Run() {
    uint64_t tick_counter = 0;
    uint64_t last_ping_tick = 0;
    auto next_tick_time = std::chrono::steady_clock::now();

    while (is_running_) {
        const auto tick_start_time = std::chrono::steady_clock::now();
        const float delta_time =
            std::chrono::duration<float>(tick_start_time - next_tick_time).count();

        const auto render_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        tick_start_time.time_since_epoch()) - kRenderDelay;

        std::cout << "[Client][DEBUG] Tick: " << tick_counter
                  << ", Render time: " << render_time.count() << " ms\n";

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                is_running_ = false;
            }

            input_manager_.HandleEvent(event, tick_start_time);
        }

        const auto packet_start_time = std::chrono::steady_clock::now();
        ProcessPackets(kMaxPacketsPerTick, kMaxPacketProcessingTime);
        const auto packet_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - packet_start_time).count();
        std::cout << "[Client][DEBUG] Packet processing time: " << packet_time << " ms\n";

        if (tick_counter - last_ping_tick >= kPingFrequencyTicks) {
            SendPing(tick_counter);
            last_ping_tick = tick_counter;
        }

        const auto update_start_time = std::chrono::steady_clock::now();
        game_engine_.Update(delta_time, render_time);
        const auto update_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - update_start_time).count();
        std::cout << "[Client][DEBUG] Update time: " << update_time << " ms\n";

        const auto render_start_time = std::chrono::steady_clock::now();
        renderer_.Clear();
        renderer_.DrawGame(game_state_);
        renderer_.Present();
        const auto render_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - render_start_time).count();
        std::cout << "[Client][DEBUG] Render time: " << render_time_ms << " ms\n";

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


/*
void Client::Run() {
  uint64_t tick_counter = 0;
  uint64_t last_ping_tick = 0;
  auto next_tick_time = std::chrono::steady_clock::now();

  while (is_running_) {
    const auto current_time = std::chrono::steady_clock::now();

    // Rattraper les ticks manqués
    while (current_time > next_tick_time) {
      const float delta_time =
          std::chrono::duration<float>(kTickDuration).count();

      SDL_Event event;
      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
          is_running_ = false;
        }
        HandleInput(event);
      }

      ProcessPackets(kMaxPacketsPerTick, kMaxPacketProcessingTime);

      if (tick_counter - last_ping_tick >= kPingFrequencyTicks) {
        SendPing(tick_counter);
        last_ping_tick = tick_counter;
      }

      game_state_.Update(delta_time);

      ++tick_counter;
      next_tick_time += kTickDuration;
    }

    renderer_.clear();
    renderer_.draw_game(game_state_);
    renderer_.present();

    const auto sleep_time = next_tick_time - std::chrono::steady_clock::now();
    if (sleep_time > std::chrono::milliseconds(0)) {
      std::this_thread::sleep_for(sleep_time);
    } else {
      std::cerr << "[Client] Tick overrun by "
                << -sleep_time.count() << " ms\n";
    }
  }
}
*/

void Client::SendPing(uint64_t tick_counter) {
  const auto timestamp = static_cast<uint32_t>(
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::steady_clock::now().time_since_epoch())
          .count());

  auto ping_packet = network::PacketFactory<network::MyPacketType>::create_packet(
      network::MyPacketType::kPing, timestamp);

  network_client_.send(std::move(ping_packet));

  std::cout << "[Client][INFO] Ping sent at tick: " << tick_counter
            << ", timestamp: " << timestamp << " ms\n";
}

void Client::ProcessPackets(const int max_packets,
                             const std::chrono::milliseconds max_time) {
  {
    const auto start_time = std::chrono::steady_clock::now();
    int processed = 0;

    while (processed < max_packets) {
      auto packet_opt = network_client_.pop_message();
      if (!packet_opt) break;

      auto current_time = std::chrono::steady_clock::now();
      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
          current_time - start_time);

      if (elapsed >= max_time) break;

      MessageDispatcher::Dispatch(std::move(packet_opt.value()), *this);
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