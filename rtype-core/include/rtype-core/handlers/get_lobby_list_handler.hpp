#ifndef HANDLE_GET_LOBBY_LIST_HPP_
#define HANDLE_GET_LOBBY_LIST_HPP_

#include <iostream>
#include <memory>
#include <optional>
#include <vector>
#include <string>
#include <utility>
#include "rtype-core/service_container.hpp"
#include "rtype-core/game_state.hpp"
#include "rtype-core/protocol.hpp"

namespace rtype {

template <typename PacketType>
void HandleGetLobbyList(const std::shared_ptr<void>& raw_event,
                        ServiceContainer& service_container,
                        GameState<PacketType>& game_state) {
    auto event = std::static_pointer_cast<
        std::pair<network::Packet<PacketType>,
                  std::shared_ptr<network::TcpServerConnection<PacketType>>>>(raw_event);

    const auto& packet = event->first;
    const auto& connection = event->second;

    // Vérifier la taille minimale du paquet
    if (packet.body.size() != sizeof(network::packets::GetLobbyListPacket)) {
        std::cerr << "[HandleGetLobbyList][ERROR] Invalid GetLobbyListPacket size." << std::endl;
        return;
    }

    // Extraire les données du paquet
    const auto* request_data = reinterpret_cast<const network::packets::GetLobbyListPacket*>(packet.body.data());
    const uint32_t offset = request_data->offset;
    const uint32_t limit = request_data->limit;
    const std::string search_term(request_data->search_term, strnlen(request_data->search_term, sizeof(request_data->search_term)));

    std::cout << "[HandleGetLobbyList] Request received with offset: " << offset
              << ", limit: " << limit
              << ", search term: '" << search_term << "'." << std::endl;

    const auto lobby_service = service_container.GetLobbyService();
    if (!lobby_service) {
        std::cerr << "[HandleGetLobbyList][ERROR] LobbyService not available." << std::endl;
        return;
    }

    // Filtrer les lobbies en fonction des critères
    const auto filtered_lobbies = lobby_service->GetLobbiesWithPagination(offset, limit, search_term);
    if (filtered_lobbies.empty()) {
        std::cout << "[HandleGetLobbyList] No lobbies found for the given criteria." << std::endl;
        auto response_packet = network::CreateGetLobbyListResponsePacket<PacketType>(
            404, {});  // 404: No lobbies found
        connection->Send(std::move(response_packet));
        return;
    }

  std::vector<network::packets::GetLobbyListResponsePacket::LobbyInfo> lobby_infos;
  for (const auto& [id, name, password_hash, game_active] : filtered_lobbies) {
    if (!game_active) {  // Exclure les lobbies en cours de jeu
      network::packets::GetLobbyListResponsePacket::LobbyInfo info = {};
      info.lobby_id = id;
      std::strncpy(info.name, name.c_str(), sizeof(info.name));
      info.has_password = password_hash.has_value();
      lobby_infos.push_back(std::move(info));
    }
  }

  if (lobby_infos.empty()) {
    std::cout << "[HandleGetLobbyList] No available lobbies found (not in-game)." << std::endl;
    auto response_packet = network::CreateGetLobbyListResponsePacket<PacketType>(
        404, {});  // 404: No lobbies found
    connection->Send(std::move(response_packet));
    return;
  }

  auto response_packet = network::CreateGetLobbyListResponsePacket<PacketType>(
        200,  // 200: Success
        lobby_infos);
  connection->Send(std::move(response_packet));

  std::cout << "[HandleGetLobbyList] Sent " << lobby_infos.size() << " lobbies in response." << std::endl;
}

}  // namespace rtype

#endif  // HANDLE_GET_LOBBY_LIST_HPP_
