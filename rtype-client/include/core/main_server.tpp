#ifndef RTYPE_CLIENT_CORE_MAIN_SERVER_TPP_
#define RTYPE_CLIENT_CORE_MAIN_SERVER_TPP_

#include <network/network_server.hpp>

#include "my_packet_types.hpp"
#include "packet_factory.hpp"
#include "scenes/game_scene.hpp"

namespace rtype {

template <typename PacketType>
MainServer<PacketType>::~MainServer() {
    Stop();
}

template <typename PacketType>
bool MainServer<PacketType>::Start(const std::string& host, const std::string& service, uint16_t udp_port)
{
    if (is_running_) {
        std::cerr << "[MainServer][ERROR] Server is already running." << std::endl;
        return false;
    }

    if (const bool connected = network_server_->Connect(host, service, udp_port);
        !connected) {
        std::cerr << "[MainServer][ERROR] Failed to connect to server." << std::endl;
        return false;
    }

    is_running_ = true;

    scene_manager_->PushScene(std::make_unique<LoginScene>());
    //try {
    //    const int tcp_port = std::stoi(service);

    //    scene_manager_->PushScene(std::make_unique<GameScene>(host, std::vector{tcp_port, static_cast<int>(udp_port)}));

    //    std::cout << "[MainServer][INFO] Successfully transitioned to GameScene." << std::endl;
    //} catch (const std::exception& e) {
    //    std::cerr << "[MainServer][ERROR] Failed to start GameScene: " << e.what() << std::endl;
    //    return false;
    //}

    Run();

    std::cout << "[MainServer][INFO] MainServer started successfully." << std::endl;
    return true;
}

template <typename PacketType>
void MainServer<PacketType>::Stop()
{
    if (!is_running_) {
        return;
    }

    is_running_ = false;

    network_server_->Disconnect();

    std::cout << "[MainServer][INFO] MainServer stopped." << std::endl;
}

template <typename PacketType>
void MainServer<PacketType>::Run() {
    using Clock = std::chrono::steady_clock;

    SDL_StartTextInput();

    constexpr int kMaxPacketsPerFrame = 200;
    constexpr auto kMaxProcessTime = std::chrono::milliseconds(5);

    // -- For the physics/logic ticks
    constexpr double kFixedTimestepMs = 15.625;  // ~64 ticks/second
    double accumulator = 0.0;

    // -- For sending periodic pings
    constexpr double kPingIntervalMs = 1000.0;
    double ping_accumulator = 0.0;

    auto previous_time = Clock::now();

    while (is_running_) {
        // 1) Poll OS events (keyboard, mouse, etc.)

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                // user clicked the [X] in the window or pressed Alt+F4
                // or some other quit event
                is_running_ = false;
                break;
            }

            scene_manager_->HandleInput(event);

            // Here, dispatch event to your Scenes/UI:
            // mySceneManager.HandleEvent(event);
            // or UI.HandleInput(event);
        }


        // 1) Measure frame time
        auto current_time = Clock::now();
        auto frame_time = current_time - previous_time;
        previous_time = current_time;

        const double delta_ms =
            std::chrono::duration<double, std::milli>(frame_time).count();
        accumulator += delta_ms;

        // 2) Accumulate time for:
        //    - fixed-step logic
        //    - ping sending
        accumulator += delta_ms;
        ping_accumulator += delta_ms;

        // 2) Process network and events
        ProcessPackets(kMaxPacketsPerFrame, kMaxProcessTime);
        event_queue_->ProcessEvents();

        // 3) Fixed update loop (may run multiple times if behind schedule)
        while (accumulator >= kFixedTimestepMs) {
            // e.g. game_logic_.FixedUpdate(kFixedTimestepMs / 1000.0);
            accumulator -= kFixedTimestepMs;

            // Optionally, enqueue parallel tasks or AI jobs here using a thread pool
            // job_system_.Enqueue(...);
        }

        // 5) Send a ping if enough *real* time has passed
        if (ping_accumulator >= kPingIntervalMs) {
            ping_accumulator -= kPingIntervalMs;

            SendPing();
        }

        // If using a job system, wait for jobs to complete:
        // job_system_.WaitForAll();

        // 3) Interpolation factor for rendering (between two logic states)
        double alpha = accumulator / kFixedTimestepMs;

        // 4) Variable update / rendering (interpolate objects with alpha)
        // renderer_.RenderScene(game_logic_, alpha);

        renderer_->Clear();
        scene_manager_->Render();
        //SDL_SetRenderDrawColor(renderer_->GetSDLRenderer(), 0, 0, 0, 255);
        renderer_->Present();

        // 5) Optional: basic frame pacing to avoid 100% CPU usage
        //SDL_Delay(5);
        //std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    SDL_StopTextInput();

    std::cout << "[MainServer][INFO] Exiting main loop." << std::endl;
}

template <typename PacketType>
void MainServer<PacketType>::SendPing() const
{
    // Get a current timestamp in ms (relative to some epoch).
    // steady_clock is often used for measuring intervals,
    // system_clock if you want "wall-clock" time.
    // Here we cast to uint32_t for the packet.
    const auto now = std::chrono::steady_clock::now();
    const uint32_t timestamp_ms = static_cast<uint32_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count()
    );

    // Create a ping packet with your PacketFactory.
    // The second parameter is the payload (timestamp).
    auto ping_packet = network::CreatePingPacket<PacketType>(timestamp_ms);

    // Send it via UDP. (Could also be TCP, depending on your protocol.)
    network_server_->SendTcp(std::move(ping_packet));

    // Log or debug output
    // std::cout << "[Client][INFO] Ping sent with timestamp: " << timestamp_ms << " ms" << std::endl;
}

template <typename PacketType>
void MainServer<PacketType>::ProcessPackets(
    const int max_packets, const std::chrono::milliseconds max_time) {
    int processed_count = 0;
    const auto start_time = std::chrono::steady_clock::now();

    while (processed_count < max_packets) {
        auto packet_opt = network_server_->PopMessage();
        if (!packet_opt) break;

        if (auto elapsed_time = std::chrono::steady_clock::now() - start_time;
            elapsed_time >= max_time) break;

        try {
            message_dispatcher_.Dispatch(std::move(packet_opt.value()));
        } catch (const std::exception& e) {
            std::cerr << "[MainServer][ERROR] Exception during message processing: " << e.what() << std::endl;
        }

        ++processed_count;
    }
}

template <typename PacketType>
void MainServer<PacketType>::HandleCommand(const std::string& cmd)
{
    if (cmd == "stop") {
        std::cout << "[MainServer] Stopping by user command." << std::endl;
        Stop();
    } else if (cmd.starts_with("register")) {
        // Command format: "register <username> <password>"
        auto args = ParseCommandArgs(cmd);
        if (args.size() == 3) {
            RegisterUser(args[1], args[2]);
        } else {
            std::cerr << "[MainServer][ERROR] Usage: register <username> <password>\n";
        }
    } else if (cmd.starts_with("login")) {
        // Command format: "login <username> <password>"
        auto args = ParseCommandArgs(cmd);
        if (args.size() == 3) {
            LoginUser(args[1], args[2]);
        } else {
            std::cerr << "[MainServer][ERROR] Usage: login <username> <password>\n";
        }
    } else if (cmd.starts_with("message")) {
        // Command format: "message <recipient_id> <message>"
        auto args = ParseCommandArgs(cmd);
        if (args.size() >= 3) {
            int recipient_id = std::stoi(args[1]);
            std::string message_content = cmd.substr(cmd.find(args[2]));

            SendMessageToPlayer(recipient_id, message_content);

        } else {
            std::cerr << "[MainServer][ERROR] Usage: message <recipient_id> <message>\n";
        }
    } else if (cmd.starts_with("create_lobby")) {
        auto args = ParseCommandArgs(cmd);
        if (args.size() >= 2) {
            CreateLobby(args[1], args.size() == 3 ? std::optional<std::string>(args[2]) : std::nullopt);
        } else {
            std::cerr << "[MainServer][ERROR] Usage: create_lobby <name> [password]\n";
        }
    } else if (cmd.starts_with("join_lobby")) {
        auto args = ParseCommandArgs(cmd);
        if (args.size() >= 2) {
            int lobby_id = std::stoi(args[1]);
            JoinLobby(lobby_id, args.size() == 3 ? std::optional<std::string>(args[2]) : std::nullopt);
        } else {
            std::cerr << "[MainServer][ERROR] Usage: join_lobby <lobby_id> [password]\n";
        }
    } else if (cmd.starts_with("ready")) {
        // Command format: "ready <is_ready>"
        auto args = ParseCommandArgs(cmd);
        if (args.size() == 2) {
            if (args[1] == "true" || args[1] == "false") {
                bool is_ready = (args[1] == "true");
                SetReadyStatus(is_ready);
            } else {
                std::cerr << "[MainServer][ERROR] Invalid value for <is_ready>. Must be 'true' or 'false'.\n";
            }
        } else {
            std::cerr << "[MainServer][ERROR] Usage: ready <true|false>\n";
        }
    } else {
        std::cout << "[MainServer] Unrecognized command: " << cmd << std::endl;
    }
}

template <typename PacketType>
void MainServer<PacketType>::RegisterUser(const std::string& username, const std::string& password)
{
    auto register_packet = network::CreateRegisterPacket<PacketType>(username, password);

    if (register_packet) {
        network_server_->SendTcp(std::move(*register_packet));
        std::cout << "[MainServer] Register request sent for username: " << username << std::endl;
    } else {
        std::cerr << "Failed to create register packet." << std::endl;
    }
}

template <typename PacketType>
void MainServer<PacketType>::LoginUser(const std::string& username, const std::string& password)
{
    auto login_packet = network::CreateLoginPacket<PacketType>(username, password);

    if (login_packet) {
        network_server_->SendTcp(std::move(*login_packet));
        std::cout << "[MainServer] Login request sent for username: " << username << std::endl;
    } else {
        std::cerr << "Failed to create login packet." << std::endl;
    }
}

template <typename PacketType>
void MainServer<PacketType>::SendMessageToPlayer(const int recipient_id, const std::string& message_content)
{
    auto message_packet = network::CreatePrivateMessagePacket<PacketType>(recipient_id, message_content);

    if (message_packet) {
        network_server_->SendTcp(std::move(*message_packet));
        std::cout << "[MainServer] Sent message to player " << recipient_id << ": " << message_content << std::endl;
    } else {
        std::cerr << "Failed to create private message packet." << std::endl;
    }
}

template <typename PacketType>
void MainServer<PacketType>::CreateLobby(const std::string& name, const std::optional<std::string>& password) {
    auto create_lobby_packet = network::CreateCreateLobbyPacket<PacketType>(name, password);

    if (!create_lobby_packet) {
        std::cerr << "[MainServer][ERROR] Failed to create CreateLobbyPacket for lobby name: " << name << std::endl;
        return;
    }

    network_server_->SendTcp(std::move(*create_lobby_packet));

    std::cout << "[MainServer] CreateLobby request sent for lobby name: " << name << std::endl;
}

template <typename PacketType>
void MainServer<PacketType>::JoinLobby(const int lobby_id, const std::optional<std::string>& password) {
    auto join_lobby_packet = network::CreateJoinLobbyPacket<PacketType>(lobby_id, password);

    if (!join_lobby_packet) {
        std::cerr << "[MainServer][ERROR] Failed to create JoinLobbyPacket for lobby ID: " << lobby_id << std::endl;
        return;
    }

    network_server_->SendTcp(std::move(*join_lobby_packet));

    std::cout << "[MainServer] JoinLobby request sent for lobby ID: " << lobby_id << std::endl;
}

template <typename PacketType>
void MainServer<PacketType>::SetReadyStatus(const bool is_ready) {
    auto ready_packet = network::CreatePlayerReadyPacket<PacketType>(is_ready);

    if (ready_packet) {
        network_server_->SendTcp(std::move(*ready_packet));
        std::cout << "[MainServer] Player  is now " << (is_ready ? "ready" : "not ready") << std::endl;
    } else {
        std::cerr << "[MainServer][ERROR] Failed to create PlayerReadyPacket.\n";
    }
}


template <typename PacketType>
std::vector<std::string> MainServer<PacketType>::ParseCommandArgs(const std::string& cmd)
{
    std::vector<std::string> args;
    std::istringstream iss(cmd);
    std::string arg;

    while (iss >> std::ws) {  // Ignore leading whitespaces
        if (iss.peek() == '"') {  // Handle quoted arguments
            iss.get();  // Consume the opening quote
            std::getline(iss, arg, '"');  // Read until the closing quote

            if (iss.fail()) {  // No closing quote found
                std::cerr << "[Error] Missing closing quote in command: " << cmd << std::endl;
                return {};  // Return empty vector to signal invalid command
            }

            args.push_back(arg);
        } else {
            iss >> arg;  // Regular argument
            args.push_back(arg);
        }
    }

    return args;
}

}

#endif
