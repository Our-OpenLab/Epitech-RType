#ifndef MAIN_SERVER_TPP_
#define MAIN_SERVER_TPP_

#include <curl/curl.h>

#include <fstream>
#include <random>

#include "handlers/ping_handlers.hpp"
#include "packet_factory.hpp"
#include "player_actions.hpp"
#include "protocol.hpp"

namespace rtype {

template <typename PacketType>
MainServer<PacketType>::MainServer(uint16_t tcp_port, uint16_t udp_port,
                                   const std::string& db_connection_string)
    : network_server_(std::make_shared<network::GameNetworkServer<PacketType>>(
          tcp_port, udp_port, event_queue_)),
      message_dispatcher_{event_queue_},
      game_state_(game_engine_.GetRegistry(), *network_server_) {

  event_queue_.Subscribe(EventType::PingTCP, [this](std::shared_ptr<void>
                                                             raw_event) {
    HandlePingTCPPacket<PacketType>(raw_event);
  });

  event_queue_.Subscribe(EventType::ClientAccepted, [this](const std::shared_ptr<void>& raw_event) {
    auto connection = std::static_pointer_cast<network::TcpServerConnection<PacketType>>(raw_event);

    std::cout << "[Server][INFO] Handling connection for client: " << connection->GetId() << std::endl;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution dis(200.0f, 1800.0f);

    const float spawn_x = dis(gen);
    const float spawn_y = dis(gen);

    if (game_state_.AddPlayer(connection->GetId(), spawn_x, spawn_y)) {
      network::packets::PlayerAssign assign_message{
          spawn_x,
          spawn_y,
        0,
        static_cast<uint8_t>(connection->GetId()),
        100
      };

    auto assign_packet = network::PacketFactory<PacketType>::CreatePacket(
        PacketType::kPlayerAssign,
        std::span(reinterpret_cast<uint8_t*>(&assign_message), sizeof(assign_message))
    );

    connection->Send(std::move(assign_packet));
    } else {
        std::cerr << "[Server][WARN] Player " << connection->GetId()
                  << " could not be added. Disconnecting." << std::endl;
        connection->Disconnect();
    }
  });

  event_queue_.Subscribe(EventType::UdpPortTCP, [this](const std::shared_ptr<void>& raw_event) {
    auto event = std::static_pointer_cast<
        std::pair<network::Packet<PacketType>,
                  std::shared_ptr<network::TcpServerConnection<PacketType>>>>(raw_event);

    const auto& packet = event->first;
    const auto& connection = event->second;

    const auto udp_info_opt = network::PacketFactory<PacketType>::template ExtractData<network::packets::UdpPortPacket>(packet);

    if (!udp_info_opt) {
        std::cerr << "[MessageDispatcher][ERROR] Invalid UDP info packet size." << std::endl;
        return;
    }

    const auto& udp_info = *udp_info_opt;
    const uint16_t udp_port = udp_info.udp_port;
    const std::string private_ip(udp_info.private_ip, sizeof(udp_info.private_ip));

    try {
        network_server_->RegisterUdpEndpoint(connection, udp_port, private_ip);

        std::cout << "[MessageDispatcher][INFO] Registered UDP port " << udp_port
                  << " and private IP " << private_ip
                  << " for client ID " << connection->GetId() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[MessageDispatcher][ERROR] Failed to register UDP endpoint: " << e.what() << std::endl;
    }
  });

  event_queue_.Subscribe(EventType::PlayerInputUDP, [this](const std::shared_ptr<void>& raw_event) {
    auto event = std::static_pointer_cast<
        std::pair<network::Packet<PacketType>, asio::ip::udp::endpoint>>(raw_event);

    const auto& packet = event->first;
    const auto& endpoint = event->second;

    const auto input_opt = network::PacketFactory<PacketType>::template ExtractData<network::packets::PlayerInputPacket>(packet);
    if (!input_opt) {
        std::cerr << "[MessageDispatcher][ERROR] Failed to extract PlayerInput data from packet.\n";
        return;
    }

    const auto& [player_id, input_actions, dir_x, dir_y] = *input_opt;

    const auto entity = game_state_.GetEntityByPlayerId(player_id);
    if (entity == GameState::InvalidEntity) {
        std::cerr << "[MessageDispatcher][ERROR] Player entity not found for player_id: "
                  << static_cast<int>(player_id) << "\n";
        return;
    }

    auto& registry = game_state_.get_registry();
    auto& actions = registry.get_components<PlayerInputState>();

    if (entity < actions.size() && actions[entity].has_value()) {
        auto& input_state = *actions[entity];
        input_state.current_actions = input_actions;
        input_state.dir_x = dir_x;
        input_state.dir_y = dir_y;
    } else {
        std::cerr << "[MessageDispatcher][WARNING] Failed to update input state for player "
                  << static_cast<int>(player_id) << ".\n";
    }
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

  game_engine_.InitializeSystems();
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

      game_engine_.Update(kFixedTimestepMs / 1000.0, game_state_, *network_server_);

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
