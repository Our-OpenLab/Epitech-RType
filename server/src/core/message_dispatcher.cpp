#include <server/core/message_dispatcher.hpp>
#include <shared/my_packet_types.hpp>
#include <shared/network_messages.hpp>
#include <shared/player_actions.hpp>

namespace game {

void MessageDispatcher::default_handler(network::Packet<network::MyPacketType>& packet, const std::shared_ptr<network::ServerConnection<network::MyPacketType>>&, GameState& game_state) {
  std::cerr << "[MessageDispatcher] Unhandled packet type: "
            << static_cast<int>(packet.header.type) << "\n";
}

void handle_ping(network::Packet<network::MyPacketType>& packet, const std::shared_ptr<network::ServerConnection<network::MyPacketType>>& connection, GameState& game_state) {
  try {
    if (packet.body.size() == sizeof(std::uint32_t)) {
      const auto timestamp = packet.extract<std::uint32_t>();
  //    std::cout << "[MessageDispatcher] Ping received with timestamp: "
  //              << timestamp << "\n";

      network::Packet<network::MyPacketType> pong_packet;
      pong_packet.header.type = network::MyPacketType::kPong;
      pong_packet.push<std::uint32_t>(timestamp);

      connection->send(pong_packet);

    //  std::cout << "[MessageDispatcher] Pong sent with timestamp: "
    //            << timestamp << "\n";
    } else {
      std::cerr << "[MessageDispatcher] Ping packet has insufficient data.\n";
    }
  } catch (const std::exception& e) {
    std::cerr << "[MessageDispatcher][ERROR] " << e.what() << "\n";
  }
}

/*

void handle_player_input(network::Packet<network::MyPacketType>& packet,
                         const std::shared_ptr<network::ServerConnection<network::MyPacketType>>& connection,
                         GameState& game_state) {
  try {
    if (packet.body.size() != sizeof(network::PlayerInput)) {
      std::cerr << "[MessageDispatcher] PlayerInput packet has insufficient data.\n";
      return;
    }

    auto input = network::PacketFactory<network::MyPacketType>::extract_data<network::PlayerInput>(packet);

    auto& registry = game_state.get_registry();
    const auto entity = game_state.get_entity_by_player_id(input.player_id);

    auto& actions = registry.get_components<Actions>();
    if (entity < actions.size() && actions[entity].has_value()) {
      actions[entity]->current_actions = input.actions;

      std::cout << "[MessageDispatcher] Player " << static_cast<int>(input.player_id)
                << " actions updated: " <<  actions[entity]->current_actions << "\n";
    }
  } catch (const std::exception& e) {
    std::cerr << "[MessageDispatcher][ERROR] Failed to process PlayerInput: " << e.what() << "\n";
  }
}

*/

/*
void HandlePlayerInput(network::Packet<network::MyPacketType>& packet,
                         const std::shared_ptr<network::ServerConnection<network::MyPacketType>>& connection,
                         GameState& game_state) {
  try {
    if (packet.body.size() != sizeof(network::PlayerInput)) {
      std::cerr
          << "[MessageDispatcher] PlayerInput packet has insufficient data.\n";
      return;
    }

    const auto input = network::PacketFactory<network::MyPacketType>::extract_data<network::PlayerInput>(packet);

    auto& registry = game_state.get_registry();
    const auto entity = game_state.get_entity_by_player_id(input.player_id);

    if (entity == static_cast<Registry::entity_t>(-1)) {
      std::cerr << "[MessageDispatcher] Player entity not found for player_id: "
                << static_cast<int>(input.player_id) << "\n";
      return;
    }

    auto& actions = registry.get_components<Actions>();
    auto& positions = registry.get_components<Position>();
    auto& last_shot_times = registry.get_components<LastShotTime>();

    if (entity < actions.size() && actions[entity].has_value()) {
      actions[entity]->current_actions = input.actions;

      // Afficher la position du joueur
      std::cout << positions[entity]->x << " " << positions[entity]->y << std::endl;

      std::cout << "[MessageDispatcher] Player " << static_cast<int>(input.player_id)
                << " actions updated: " << actions[entity]->current_actions << "\n";

      if (input.actions & static_cast<uint16_t>(PlayerAction::Shoot)) {
        if (entity < last_shot_times.size() && last_shot_times[entity].has_value() &&
            entity < positions.size() && positions[entity].has_value()) {

          auto& last_shot_time = last_shot_times[entity]->last_shot_time;
          const auto current_time = std::chrono::milliseconds(input.timestamp);

          if (current_time - last_shot_time >= std::chrono::milliseconds(200)) {
            last_shot_time = current_time;

            const auto x = positions[entity]->x;
            const auto y = positions[entity]->y;

            uint8_t projectile_id = game_state.generate_projectile_id();
            const auto projectile_entity = registry.spawn_entity();

            registry.emplace_component<Projectile>(projectile_entity, Projectile{input.player_id, projectile_id});
            registry.emplace_component<Position>(projectile_entity, Position{x, y});
            registry.emplace_component<Velocity>(projectile_entity, Velocity{0.0f, -300.0f});
            registry.emplace_component<DirtyFlag>(projectile_entity, DirtyFlag{true});

            game_state.add_projectile(projectile_id, input.player_id, projectile_entity);

            std::cout << "[MessageDispatcher] Player " << static_cast<int>(input.player_id)
                      << " fired a projectile with ID " << static_cast<int>(projectile_id)
                      << " from position (" << x << ", " << y << ").\n";
          }
        }
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "[MessageDispatcher][ERROR] Failed to process PlayerInput: " << e.what() << "\n";
  }
}
*/

void HandlePlayerInput(network::Packet<network::MyPacketType>& packet,
                         const std::shared_ptr<network::ServerConnection<network::MyPacketType>>& connection,
                         GameState& game_state) {
  try {
    if (packet.body.size() != sizeof(network::PlayerInput)) {
      std::cerr << "[MessageDispatcher] PlayerInput packet has insufficient data.\n";
      return;
    }

    auto input = network::PacketFactory<network::MyPacketType>::extract_data<network::PlayerInput>(packet);

    const auto entity = game_state.GetEntityByPlayerId(input.player_id);

    if (entity == static_cast<Registry::entity_t>(-1)) {
      std::cerr << "[MessageDispatcher] Player entity not found for player_id: "
                << static_cast<int>(input.player_id) << "\n";
      connection->disconnect();
      return;
    }

    auto& registry = game_state.get_registry();
    auto& actions = registry.get_components<Actions>();
    auto& positions = registry.get_components<Position>();
    auto& last_shot_times = registry.get_components<LastShotTime>();

    if (entity < actions.size() && actions[entity].has_value()) {
      actions[entity]->current_actions = input.actions;

      if (input.actions & static_cast<uint16_t>(PlayerAction::Shoot)) {
        if (entity < last_shot_times.size() && last_shot_times[entity].has_value() &&
            entity < positions.size() && positions[entity].has_value()) {

          auto& last_shot_time = last_shot_times[entity]->last_shot_time;

          if (const auto current_time =
                  std::chrono::milliseconds(input.timestamp);
              current_time - last_shot_time >= std::chrono::milliseconds(200)) {
            last_shot_time = current_time;

            const auto x = positions[entity]->x;
            const auto y = positions[entity]->y;

            game_state.AddProjectile(input.player_id, x, y);

            std::cout << "[MessageDispatcher] Player " << static_cast<int>(input.player_id)
                      << " fired a projectile from position (" << x << ", " << y << ").\n";
          }
        }
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "[MessageDispatcher][ERROR] Failed to process PlayerInput: " << e.what() << "\n";
  }
}

const std::array<MessageDispatcher::Handler, static_cast<size_t>(network::MyPacketType::kMaxTypes)> MessageDispatcher::handlers_ = {
  default_handler,
  HandlePlayerInput,
  default_handler,
  handle_ping,
  default_handler,
  //handle_player_input,
  default_handler, // UpdatePosition
  default_handler, // PlayerAssign
};
}

/*
enum class MyPacketType : uint32_t {
  kPlayerAssign,       // Server -> Client: Assign an ID to the player
  kPlayerInput,        // Client -> Server: Player's input data
  kUpdatePosition,     // Server -> Client: Updated position of a player
  kUpdateProjectile,   // Server -> Client: Updated position of a projectile
  kPlayerJoin,         // Server -> Client: Notification of a new player joining
  kPlayerLeave,        // Server -> Client: Notification of a player leaving
  kDisconnect,         // Client -> Server: Player's voluntary disconnection
  kPing,               // Ping packet
  kPong,               // Pong packet
  kMaxTypes            // Maximum number of packet types
};
}
*/