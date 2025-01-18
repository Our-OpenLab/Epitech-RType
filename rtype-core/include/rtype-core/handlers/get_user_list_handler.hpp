#ifndef GET_USER_LIST_HANDLER_HPP_
#define GET_USER_LIST_HANDLER_HPP_

#include <iostream>
#include <memory>
#include <utility>
#include <vector>

#include "rtype-core/service_container.hpp"
#include "rtype-core/game_state.hpp"
#include "rtype-core/protocol.hpp"

namespace rtype {

template <typename PacketType>
void HandleGetUserList(const std::shared_ptr<void>& raw_event,
                       ServiceContainer& service_container,
                       GameState<PacketType>& game_state) {
    auto event = std::static_pointer_cast<
        std::pair<network::Packet<PacketType>,
                  std::shared_ptr<network::TcpServerConnection<PacketType>>>>(raw_event);

    const auto& packet = event->first;
    const auto& connection = event->second;

    // Vérification de la taille du paquet
    if (packet.body.size() != sizeof(network::packets::GetUserListPacket)) {
        std::cerr << "[GetUserListHandler][ERROR] Invalid GetUserListPacket size." << std::endl;
        return;
    }

    // Vérification de l'autorisation du client
    if (const int connection_id = connection->GetId();!game_state.IsPlayerActiveByConnectionId(connection_id)) {
        std::cerr << "[GetUserListHandler][ERROR] Client is not authorized. Connection ID: " << connection_id << std::endl;
        return;
    }

    // Extraction de la requête
    const auto* request = reinterpret_cast<const network::packets::GetUserListPacket*>(packet.body.data());
    const auto offset = request->offset;
    const auto limit = request->limit;

    // Vérification du service utilisateur
    const auto user_service = service_container.GetUserService();
    if (!user_service) {
        std::cerr << "[GetUserListHandler][ERROR] UserService not available." << std::endl;
        return;
    }

    // Récupération des utilisateurs
    const auto users = user_service->GetUsers(offset, limit);
    if (users.empty()) {
        std::cerr << "[GetUserListHandler][INFO] No users found for offset " << offset << " and limit " << limit << "." << std::endl;
        auto response = network::CreateGetUserListResponsePacket<PacketType>(404, {});
        connection->Send(std::move(response));
        return;
    }

    // Préparation des données pour la réponse
    std::vector<network::packets::GetUserListResponsePacket::UserInfo> user_info_list;
    for (const auto& user : users) {
        network::packets::GetUserListResponsePacket::UserInfo user_info{};
        user_info.user_id = user.id;
        std::memcpy(user_info.username, user.username.c_str(), std::min(user.username.size(), sizeof(user_info.username)));
        user_info.is_online = game_state.IsPlayerActiveByDbId(user.id);
        user_info_list.push_back(user_info);
    }

    // Création et envoi du paquet de réponse
    auto response = network::CreateGetUserListResponsePacket<PacketType>(200, user_info_list);
    connection->Send(std::move(response));

    std::cout << "[GetUserListHandler] Sent user list response with " << users.size() << " users." << std::endl;
}

}  // namespace rtype

#endif  // GET_USER_LIST_HANDLER_HPP_
