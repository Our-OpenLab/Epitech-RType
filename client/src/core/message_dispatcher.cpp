#include <iostream>

#include "client/core/message_dispatcher.hpp"
#include "client/core/client.hpp"

void MessageDispatcher::Dispatch(network::Packet<network::MyPacketType>&& packet, Client& client) {
  const auto index = static_cast<size_t>(packet.header.type);

  if (index >= handlers_.size() || !handlers_[index]) {
    DefaultHandler(packet, client);
    return;
  }
  handlers_[index](packet, client);
}

void MessageDispatcher::DefaultHandler(network::Packet<network::MyPacketType>& packet, Client& client) {
  std::cout << "[Client][WARNING] Unhandled packet type: " << static_cast<int>(packet.header.type) << "\n";
}

void MessageDispatcher::HandlePlayerAssign(const network::Packet<network::MyPacketType>& packet, Client& client) {
  const auto [player_id] = network::PacketFactory<network::MyPacketType>::extract_data<network::PlayerAssign>(packet);

  client.client_id_ = player_id;

  std::cout << "[Server][INFO] Assigned Player ID: " << static_cast<int>(client.client_id_) << std::endl;

 // if (!client.GetGameState().AddPlayer(client.client_id_, x, y)) {
 //   std::cerr << "[Server][ERROR] Failed to add Player ID: " << static_cast<int>(client.client_id_)
 //             << " to GameState. Player might already exist.\n";
 //
 //   client.Shutdown();
 //   return;
 // }

  std::cout << "[Server][INFO] Player " << static_cast<int>(client.client_id_)
            << " successfully added to GameState.\n";
}

void MessageDispatcher::HandleUpdatePositions(const network::Packet<network::MyPacketType>& packet, Client& client) {
  const auto update_positions = network::PacketFactory<network::MyPacketType>::extract_data_array<network::UpdatePosition>(packet);
  auto& game_state = client.GetGameState();
  auto& registry = game_state.GetRegistry();
  auto& positions = registry.get_components<Position>();

  for (const auto& [player_id, new_x, new_y, _] : update_positions) {
    if (const auto entity = game_state.GetPlayer(player_id);
        entity == client::GameState::InvalidEntity) {
      std::cout << "[Client][INFO] Adding new player with ID: " << static_cast<int>(player_id) << '\n';

      game_state.AddPlayer(player_id, new_x, new_y);

      std::cout << "[Client][INFO] Added Player " << static_cast<int>(player_id)
                << " at position (" << new_x << ", " << new_y << ")\n";
    } else if (entity < positions.size() && positions[entity].has_value()) {
      auto& [x, y] = *positions[entity];
      x = new_x;
      y = new_y;

      std::cout << "[Client][INFO] Updated position for Player " << static_cast<int>(player_id)
                << " to (" << new_x << ", " << new_y << ")\n";
    } else {
      std::cerr << "[Client][WARNING] Position component not found for Player ID: "
                << static_cast<int>(player_id) << '\n';
    }
  }
}

void MessageDispatcher::HandleUpdateProjectiles(const network::Packet<network::MyPacketType>& packet, Client& client) {
  const auto update_projectiles = network::PacketFactory<network::MyPacketType>::extract_data_array<network::UpdateProjectile>(packet);
  auto& game_state = client.GetGameState();
  auto& registry = game_state.GetRegistry();
  auto& positions = registry.get_components<Position>();

  for (const auto& [projectile_id, owner_id, new_x, new_y, _] : update_projectiles) {
    if (const auto entity = game_state.GetProjectileEntity(projectile_id);
        entity == client::GameState::InvalidEntity) {
      std::cout << "[Client][INFO] Adding new projectile with ID: " << projectile_id
                << " for Owner " << static_cast<int>(owner_id) << " at position ("
                << new_x << ", " << new_y << ")\n";

      game_state.AddProjectile(projectile_id, owner_id, new_x, new_y);
        } else if (entity < positions.size() && positions[entity].has_value()) {
          auto& [x, y] = *positions[entity];
          x = new_x;
          y = new_y;

          std::cout << "[Client][INFO] Updated position for projectile " << projectile_id
                    << " of Owner " << static_cast<int>(owner_id) << " to ("
                    << new_x << ", " << new_y << ")\n";
        } else {
          std::cerr << "[Client][WARNING] Position component not found for projectile ID: "
                    << projectile_id << '\n';
        }
  }
}

void MessageDispatcher::HandlePong(network::Packet<network::MyPacketType>& packet, Client& client) {
  try {
    if (packet.body.size() == sizeof(std::uint32_t)) {
      const auto timestamp = packet.extract<std::uint32_t>();

      const auto current_time = std::chrono::steady_clock::now();
      const auto received_time = std::chrono::milliseconds(timestamp);

      const auto ping = std::chrono::duration_cast<std::chrono::milliseconds>(
                            current_time.time_since_epoch())
                            .count() -
                        received_time.count();

      std::cout << "[MessageDispatcher][INFO] Ping received: " << ping << " ms\n";
    } else {
      std::cerr << "[MessageDispatcher][ERROR] Pong packet has insufficient data.\n";
    }
  } catch (const std::exception& e) {
    std::cerr << "[MessageDispatcher][ERROR] Exception while handling Pong: " << e.what() << '\n';
  }
}


/*
void MessageDispatcher::HandleUpdatePositions(const network::Packet<network::MyPacketType>& packet, Client& client) {
  try {
    auto update_positions = network::PacketFactory<network::MyPacketType>::extract_data_array<network::UpdatePosition>(packet);

    auto& game_state = client.GetGameState();

    for (const auto& update_position : update_positions) {
      try {
        auto entity = game_state.GetPlayer(update_position.player_id);
        auto& registry = game_state.GetRegistry();
        auto& positions = registry.get_components<Position>();

        if (entity < positions.size() && positions[entity].has_value()) {
          auto& position = *positions[entity];
          position.x = update_position.x;
          position.y = update_position.y;

          std::cout << "[Client][INFO] Updated position for Player "
                    << static_cast<int>(update_position.player_id)
                    << " to (" << position.x << ", " << position.y << ")\n";
        } else {
          std::cerr << "[Client][WARNING] Position component not found for Player ID: "
                    << static_cast<int>(update_position.player_id) << '\n';
        }
      } catch (const std::exception& e) {
        std::cerr << "[MessageDispatcher][WARNING] Failed to update position for Player ID: "
                  << static_cast<int>(update_position.player_id) << ". Reason: " << e.what() << '\n';
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "[MessageDispatcher][ERROR] Exception while handling UpdatePositions: " << e.what() << '\n';
  }
}
*/


/*
void MessageDispatcher::HandleUpdatePositions(const network::Packet<network::MyPacketType>& packet, Client& client) {
    try {
        auto update_positions = network::PacketFactory<network::MyPacketType>::extract_data_array<network::UpdatePosition>(packet);

        auto& game_state = client.GetGameState();

        for (const auto& update_position : update_positions) {
            try {
                auto entity = game_state.GetPlayer(update_position.player_id);
                auto& registry = game_state.GetRegistry();
                auto& positions = registry.get_components<Position>();
                auto& histories = registry.get_components<PositionHistory>();

                if (entity < positions.size() && positions[entity].has_value()) {
                    auto& position = *positions[entity];
                    auto& history = histories[entity];

                    // Ajoute la position avec le timestamp serveur
                    const auto server_timestamp = std::chrono::milliseconds(update_position.timestamp);

                    // Ajoute une nouvelle snapshot à l'historique
                    if (history.has_value()) {
                        history->AddSnapshot({update_position.x, update_position.y}, server_timestamp);
                    }

                    std::cout << "[Client][INFO] Updated position for Player "
                              << static_cast<int>(update_position.player_id)
                              << " to (" << update_position.x << ", " << update_position.y
                              << ") at timestamp " << update_position.timestamp << "\n";
                } else {
                    std::cerr << "[Client][WARNING] Position component not found for Player ID: "
                              << static_cast<int>(update_position.player_id) << '\n';
                }
            } catch (const std::exception& e) {
                std::cerr << "[MessageDispatcher][WARNING] Failed to update position for Player ID: "
                          << static_cast<int>(update_position.player_id) << ". Reason: " << e.what() << '\n';
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[MessageDispatcher][ERROR] Exception while handling UpdatePositions: " << e.what() << '\n';
    }
}
*/

/*

void MessageDispatcher::HandleUpdatePositions(
    const network::Packet<network::MyPacketType>& packet, Client& client) {
    try {
        auto update_positions = network::PacketFactory<network::MyPacketType>::extract_data_array<network::UpdatePosition>(packet);

        auto& game_state = client.GetGameState();

        for (const auto& update_position : update_positions) {
            try {
                auto entity = game_state.GetPlayer(update_position.player_id);
                auto& registry = game_state.GetRegistry();
                auto& positions = registry.get_components<Position>();
                auto& histories = registry.get_components<PositionHistory>();

                if (entity < positions.size() && positions[entity].has_value()) {
                    auto& position = *positions[entity];
                    auto& history = histories[entity];

                    const auto server_timestamp = std::chrono::milliseconds(update_position.timestamp);

                    if (history.has_value()) {
                        history->AddSnapshot({update_position.x, update_position.y}, server_timestamp);
                    }

                    std::cout << "[MessageDispatcher][DEBUG] Received update for Player "
                              << static_cast<int>(update_position.player_id)
                              << ": Position(" << update_position.x << ", " << update_position.y
                              << "), Timestamp: " << update_position.timestamp << " ms\n";
                } else {
                    std::cerr << "[MessageDispatcher][WARNING] Position component not found for Player ID: "
                              << static_cast<int>(update_position.player_id) << '\n';
                }
            } catch (const std::exception& e) {
                std::cerr << "[MessageDispatcher][WARNING] Failed to update position for Player ID: "
                          << static_cast<int>(update_position.player_id) << ". Reason: " << e.what() << '\n';
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[MessageDispatcher][ERROR] Exception while handling UpdatePositions: " << e.what() << '\n';
    }
}

*/

void MessageDispatcher::HandlePlayerJoin(const network::Packet<network::MyPacketType>& packet, Client& client) {
  auto [player_id, x, y] = network::PacketFactory<network::MyPacketType>::extract_data<network::PlayerJoin>(packet);

  std::cout << "[Client][INFO] Player " << static_cast<int>(player_id)
            << " joined the game at position (" << x << ", " << y << ").\n";

  client.GetGameState().AddPlayer(player_id, x, y);

  std::cout << "[Client][INFO] Player " << static_cast<int>(player_id)
            << " successfully added to local GameState.\n";
}


void MessageDispatcher::HandlePlayerLeave(const network::Packet<network::MyPacketType>& packet, Client& client) {
  auto [player_id] = network::PacketFactory<network::MyPacketType>::extract_data<network::PlayerLeave>(packet);

  std::cout << "[Client][INFO] Player " << static_cast<int>(player_id) << " has left the game.\n";

  client.GetGameState().RemovePlayer(player_id);

  std::cout << "[Client][INFO] Player " << static_cast<int>(player_id)
            << " successfully removed locally.\n";
}

void MessageDispatcher::HandleRemoveProjectiles(
  const network::Packet<network::MyPacketType>& packet, Client& client) {
  const auto [projectile_id] =
      network::PacketFactory<network::MyPacketType>::extract_data<network::RemoveProjectile>(packet);

  auto& game_state = client.GetGameState();
  game_state.RemoveProjectile(projectile_id);
}


const std::array<MessageDispatcher::Handler, static_cast<size_t>(network::MyPacketType::kMaxTypes)> MessageDispatcher::handlers_ = {
  HandlePlayerAssign,     // PlayerAssign
  DefaultHandler,         // PlayerInput
  HandleUpdatePositions,  // UpdatePosition
  HandleUpdateProjectiles, // UpdateProjectile
  HandleRemoveProjectiles,          // Ping
  HandlePlayerJoin,       // PlayerJoin
  HandlePlayerLeave,       // PlayerLeave
  HandleUpdateProjectiles,
         // PlayerAssign
};


/*
namespace network {
enum class MyPacketType : uint32_t {
  kPlayerAssign,       // Server -> Client: Assign an ID to the player
  kPlayerInput,        // Client -> Server: Player's input data
  kUpdatePosition,     // Server -> Client: Updated position of a player
  kUpdateProjectile,   // Server -> Client: Updated position of a projectile
  kRemoveProjectile,   // Server -> Client: Remove a projectile
  kPlayerJoin,         // Server -> Client: Notification of a new player joining
  kPlayerLeave,        // Server -> Client: Notification of a player leaving
  kDisconnect,         // Client -> Server: Player's voluntary disconnection
  kPing,               // Ping packet
  kPong,               // Pong packet
  kMaxTypes            // Maximum number of packet types
};
}
*/
