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

void MessageDispatcher::HandlePlayerAssign(const network::Packet<network::MyPacketType>& packet, Client& client) {
  auto assign_message = network::PacketFactory<network::MyPacketType>::extract_data<network::PlayerAssign>(packet);
  client.client_id_ = assign_message.player_id;

  std::cout << "[Client][INFO] Assigned Player ID: " << static_cast<int>(client.client_id_) << "\n";

  client.GetGameState().AddPlayer(assign_message.player_id, 100.0f, 100.0f);
}


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

void MessageDispatcher::HandleUpdateProjectiles(const network::Packet<network::MyPacketType>& packet, Client& client) {
  try {
    auto update_projectiles = network::PacketFactory<network::MyPacketType>::extract_data_array<network::UpdateProjectile>(packet);

    auto& game_state = client.GetGameState();
    auto& registry = game_state.GetRegistry();

    for (const auto& update_projectile : update_projectiles) {
      try {
        // Obtenir l'entité associée au projectile
        auto entity = game_state.GetProjectileEntity(update_projectile.projectile_id);

        if (entity == static_cast<Registry::entity_t>(-1)) {
          // Ajouter le projectile au GameState
          game_state.AddProjectile(update_projectile.projectile_id, update_projectile.owner_id, update_projectile.x, update_projectile.y);

          std::cout << "[Client][INFO] Created new projectile with ID " << update_projectile.projectile_id
                    << " for Owner " << static_cast<int>(update_projectile.owner_id) << " at ("
                    << update_projectile.x << ", " << update_projectile.y << ")\n";
        } else {
          // Si le projectile existe, mettre à jour sa position
          auto& positions = registry.get_components<Position>();
          if (entity < positions.size() && positions[entity].has_value()) {
            auto& position = *positions[entity];
            position.x = update_projectile.x;
            position.y = update_projectile.y;

            std::cout << "[Client][INFO] Updated position for projectile " << update_projectile.projectile_id
                      << " of Owner " << static_cast<int>(update_projectile.owner_id) << " to ("
                      << position.x << ", " << position.y << ")\n";
          } else {
            std::cerr << "[Client][WARNING] Position component not found for projectile ID: "
                      << update_projectile.projectile_id << '\n';
          }
        }
      } catch (const std::exception& e) {
        std::cerr << "[MessageDispatcher][WARNING] Failed to update projectile ID: "
                  << update_projectile.projectile_id << ". Reason: " << e.what() << '\n';
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "[MessageDispatcher][ERROR] Exception while handling UpdateProjectiles: " << e.what() << '\n';
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

void MessageDispatcher::DefaultHandler(network::Packet<network::MyPacketType>& packet, Client& client) {
  std::cout << "[Client][WARNING] Unhandled packet type: " << static_cast<int>(packet.header.type) << "\n";
}

const std::array<MessageDispatcher::Handler, static_cast<size_t>(network::MyPacketType::kMaxTypes)> MessageDispatcher::handlers_ = {
  DefaultHandler,          // ChatMessage
  DefaultHandler,          // PlayerMove
  DefaultHandler,          // Disconnect
  DefaultHandler,          // Ping
  HandlePong,              // Pong
  DefaultHandler,          // PlayerInput
  HandleUpdatePositions,   // UpdatePosition
  HandleUpdateProjectiles,
  HandlePlayerAssign       // PlayerAssign
};
