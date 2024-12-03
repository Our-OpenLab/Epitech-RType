#include <SDL.h>
#include <iostream>

#include <client/core/client.hpp>
#include <client/core/message_dispatcher.hpp>
#include <client/core/ping_manager.hpp>
#include <shared/player_actions.hpp>

Client::Client(const std::string& host, const std::string& port)
    : renderer_(800, 600, "R-Type"), network_client_(host, port), ping_manager_(*this) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
    throw std::runtime_error("SDL_Init failed: " + std::string(SDL_GetError()));
  }

  std::cout << "[Client][INFO] Initialization completed successfully." << std::endl;
}

Client::~Client() {
  shutdown();
}

void Client::run() {
  constexpr auto tick_duration = std::chrono::milliseconds(16);

  time_manager_.start_tick(tick_duration);

  while (is_running_) {
    time_manager_.update();

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        is_running_ = false;
      }
      handle_input(event);
    }

    process_packets(50, std::chrono::milliseconds(10));

    ping_manager_.update();

    //game_state_.update(time_manager_.delta_time_ms().count());

    renderer_.clear();
    renderer_.draw_game(game_state_);
    renderer_.present();

    time_manager_.wait_for_next_tick();
  }
}

void Client::handle_input(const SDL_Event& event) {
  using namespace player_actions;

  static uint16_t current_actions_ = 0;
  static std::chrono::steady_clock::time_point last_input_sent_ = std::chrono::steady_clock::now();

  bool actions_changed = false;

  if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
    const bool key_down = (event.type == SDL_KEYDOWN);

    switch (event.key.keysym.sym) {
      case SDLK_w:
        if (key_down) actions_changed |= !(current_actions_ & PlayerAction::MoveUp);
        current_actions_ = key_down ? (current_actions_ | PlayerAction::MoveUp)
                                    : (current_actions_ & ~PlayerAction::MoveUp);
        break;
      case SDLK_s:
        if (key_down) actions_changed |= !(current_actions_ & PlayerAction::MoveDown);
        current_actions_ = key_down ? (current_actions_ | PlayerAction::MoveDown)
                                    : (current_actions_ & ~PlayerAction::MoveDown);
        break;
      case SDLK_a:
        if (key_down) actions_changed |= !(current_actions_ & PlayerAction::MoveLeft);
        current_actions_ = key_down ? (current_actions_ | PlayerAction::MoveLeft)
                                    : (current_actions_ & ~PlayerAction::MoveLeft);
        break;
      case SDLK_d:
        if (key_down) actions_changed |= !(current_actions_ & PlayerAction::MoveRight);
        current_actions_ = key_down ? (current_actions_ | PlayerAction::MoveRight)
                                    : (current_actions_ & ~PlayerAction::MoveRight);
        break;
      case SDLK_SPACE:
        if (key_down) actions_changed |= !(current_actions_ & PlayerAction::Shoot);
        current_actions_ = key_down ? (current_actions_ | PlayerAction::Shoot)
                                    : (current_actions_ & ~PlayerAction::Shoot);
        break;
      default:
        break;
    }
  }

  const auto now = std::chrono::steady_clock::now();
  const auto time_since_last_packet = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_input_sent_);

  if (actions_changed || time_since_last_packet >= std::chrono::milliseconds(50)) {
    const auto timestamp =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            time_manager_.now().time_since_epoch())
            .count();

    const network::PlayerInput input{
      .player_id = client_id_,
      .actions = current_actions_,
      .timestamp = static_cast<uint32_t>(timestamp)
    };

    auto input_packet = network::PacketFactory<network::MyPacketType>::create_packet(
        network::MyPacketType::PlayerInput, input);
    network_client_.send(std::move(input_packet));

    last_input_sent_ = now;
  }
}


void Client::process_packets(const int max_packets,
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

      MessageDispatcher::dispatch(std::move(packet_opt.value()), *this);
      ++processed;
    }
  }
}


void Client::shutdown() {
  std::cout << "[Client][INFO] Shutting down client..." << std::endl;

  renderer_.shutdown();
  SDL_Quit();

  std::cout << "[Client][INFO] Shutdown completed." << std::endl;
}