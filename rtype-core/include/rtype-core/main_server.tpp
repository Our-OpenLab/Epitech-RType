#ifndef MAIN_SERVER_TPP_
#define MAIN_SERVER_TPP_

#include <curl/curl.h>

#include <fstream>

#include "packet_factory.hpp"
#include "protocol.hpp"

#include "handlers/user_register_handler.hpp"
#include "handlers/user_login_handler.hpp"
#include "handlers/private_message_handler.hpp"
#include "handlers/create_lobby_handler.hpp"
#include "handlers/player_ready_handler.hpp"
#include "handlers/get_user_list_handler.hpp"
#include "handlers/private_chat_history_handler.hpp"

namespace rtype {

template <typename PacketType>
MainServer<PacketType>::MainServer(uint16_t tcp_port, uint16_t udp_port,
                                   const std::string& db_connection_string)
    : game_state_(event_queue_),
      network_server_(std::make_shared<network::GameNetworkServer<PacketType>>(
          tcp_port, udp_port, event_queue_)),
      message_dispatcher_{event_queue_},
      service_container_(db_connection_string) {

  event_queue_.Subscribe(EventType::UserRegister, [this](std::shared_ptr<void>
                                                             raw_event) {
    HandleUserRegister<PacketType>(raw_event, service_container_);
  });

  event_queue_.Subscribe(EventType::UserLogin, [this](std::shared_ptr<void>
                                                          raw_event) {
    HandleUserLogin<PacketType>(raw_event, service_container_, game_state_);
  });

  event_queue_.Subscribe(EventType::PrivateMessage, [this](std::shared_ptr<void>
                                                               raw_event) {
    HandlePrivateMessage<PacketType>(raw_event, service_container_, game_state_);
  });

  event_queue_.Subscribe(EventType::CreateLobby, [this](std::shared_ptr<void>
                                                            raw_event) {
    HandleCreateLobby<PacketType>(raw_event, service_container_, game_state_);
  });

  event_queue_.Subscribe(EventType::PlayerReady, [this](std::shared_ptr<void>
                                                            raw_event) {
    HandlePlayerReady<PacketType>(raw_event, service_container_, game_state_);
  });

  event_queue_.Subscribe(EventType::GetUserList, [this](std::shared_ptr<void>
                                                            raw_event) {
    HandleGetUserList<PacketType>(raw_event, service_container_, game_state_);
  });

  event_queue_.Subscribe(EventType::PrivateChatHistory, [this](std::shared_ptr<void>
                                                               raw_event) {
    HandlePrivateChatHistory<PacketType>(raw_event, service_container_, game_state_);
  });
}

template <typename PacketType>
MainServer<PacketType>::~MainServer() {
  Stop();
}

template <typename PacketType>
bool MainServer<PacketType>::Start() {
  if (!network_server_->Start()) {
    std::cerr << "[MainServer] Failed to start network server.\n";
    return false;
  }

  // game_engine_.InitializeSystems();
  is_running_ = true;

  game_thread_ = std::thread([this]() { Run(); });
  std::cout << "[MainServer] Server started successfully.\n";
  return true;
}

template <typename PacketType>
void MainServer<PacketType>::Stop() {
  if (!is_running_) return;

  is_running_ = false;
  network_server_->Stop();

  if (game_thread_.joinable()) {
    game_thread_.join();
  }

  std::cout << "[MainServer] Server stopped.\n";
}

template <typename PacketType>
void MainServer<PacketType>::Run() {
  using Clock = std::chrono::steady_clock;

  // Adjust these based on server load and desired tick rate
  constexpr int kMaxPacketsPerFrame = 200;
  constexpr auto kMaxProcessTime = std::chrono::milliseconds(5);

  // Fixed timestep for server logic (e.g. ~20ms => 50 updates/sec or ~15.625ms
  // => 64 updates/sec)
  constexpr double kFixedTimestepMs = 15.625;
  double accumulator = 0.0;

  auto previous_time = Clock::now();

  while (is_running_) {
    auto current_time = Clock::now();
    auto frame_time = current_time - previous_time;
    previous_time = current_time;

    // Convert frame time to milliseconds
    const double delta_ms =
        std::chrono::duration<double, std::milli>(frame_time).count();
    accumulator += delta_ms;

    // 1) Process incoming network packets (from clients) in a thread-safe queue
    ProcessPackets(kMaxPacketsPerFrame, kMaxProcessTime);

    // 2) Process any pending events (e.g., scheduled server tasks)
    event_queue_.ProcessEvents();

    // 3) Fixed-update loop
    while (accumulator >= kFixedTimestepMs) {
      // Update authoritative server logic
      // game_logic_.FixedUpdate(kFixedTimestepMs / 1000.0);

      // Decrement accumulator
      accumulator -= kFixedTimestepMs;

      // Optionally enqueue parallel tasks (AI, pathfinding, etc.) in a job
      // system job_system_.Enqueue(...);
    }

    // If using a job system, wait for tasks to finish before the next loop
    // job_system_.WaitForAll();

    // 4) (Optional) If you have some interpolation or partial updates, compute
    // alpha For a pure server, you might not need interpolation. Shown here for
    // completeness:
    double alpha = accumulator / kFixedTimestepMs;

    // 5) (Optional) A server normally doesn't render, but you could do
    // additional
    //    "variable-step" tasks here (e.g., sending periodic snapshots to
    //    clients)

    // 6) Basic pacing: sleep briefly to avoid 100% CPU usage if your logic is
    // light
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  std::cout << "[MainServer][INFO] Exiting main loop." << std::endl;
}

template <typename PacketType>
void MainServer<PacketType>::ProcessPackets(
    const int max_packets, const std::chrono::milliseconds max_time) {
  int processed_packets = 0;
  const auto start_time = std::chrono::steady_clock::now();

  while (processed_packets < max_packets) {
    auto packet_opt = network_server_->PopMessage();
    if (!packet_opt) break;

    if (auto elapsed_time = std::chrono::steady_clock::now() - start_time;
        elapsed_time >= max_time)
      break;

    try {
      message_dispatcher_.Dispatch(std::move(packet_opt.value()));
    } catch (const std::exception& e) {
      std::cerr << "[MainServer][ERROR] Exception during message processing: "
                << e.what() << std::endl;
    }

    ++processed_packets;
  }
}

}  // namespace rtype

#endif  // MAIN_SERVER_TPP_
