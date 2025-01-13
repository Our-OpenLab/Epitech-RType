#ifndef USER_LOGIN_HANDLER_HPP_
#define USER_LOGIN_HANDLER_HPP_

#include <iostream>
#include <memory>
#include <string>
#include <utility>

#include "rtype-core/game_state.hpp"
#include "rtype-core/protocol.hpp"
#include "rtype-core/service_container.hpp"

namespace rtype {
template <typename PacketType>
void HandleUserLogin(const std::shared_ptr<void>& raw_event,
                     ServiceContainer& service_container,
                     GameState<PacketType>& game_state) {
  auto event = std::static_pointer_cast<
      std::pair<network::Packet<PacketType>,
                std::shared_ptr<network::TcpServerConnection<PacketType>>>>(
      raw_event);

  const auto& packet = event->first;
  const auto& connection = event->second;

  if (packet.body.size() != sizeof(network::packets::LoginPacket)) {
    std::cerr << "[UserLoginHandler][ERROR] Invalid LoginPacket size."
              << std::endl;
    auto response_packet = network::CreateLoginResponsePacket<PacketType>(
        400);  // Code 400: Invalid request
    connection->Send(std::move(response_packet));
    return;
  }

  if (const int sender_id = connection->GetId();
      game_state.IsPlayerActive(sender_id)) {
    std::cerr << "[UserLoginHandler][ERROR] Sender is already connected: ID "
              << sender_id << std::endl;
    auto response_packet = network::CreateLoginResponsePacket<PacketType>(
        403);  // Code 403: Already connected
    connection->Send(std::move(response_packet));
    return;
  }

  const auto* login_data =
      reinterpret_cast<const network::packets::LoginPacket*>(
          packet.body.data());
  const std::string username(login_data->username);
  const std::string password(login_data->password);

  if (const auto user_service = service_container.GetUserService()) {
    if (const auto user_id =
            user_service->AuthenticateUser(username, password)) {
      std::cout << "[UserLoginHandler] User login successful: " << username
                << " (ID: " << *user_id << ")" << std::endl;

      game_state.AddPlayer(connection->GetId(), connection);

      auto response_packet = network::CreateLoginResponsePacket<PacketType>(200);  // Code 200: Success
      connection->Send(std::move(response_packet));
    } else {
      std::cerr << "[UserLoginHandler][ERROR] Authentication failed for user: " << username << std::endl;

      auto response_packet = network::CreateLoginResponsePacket<PacketType>(401);  // Code 401: Unauthorized
      connection->Send(std::move(response_packet));
    }
  } else {
    std::cerr << "[UserLoginHandler][ERROR] UserService not available."
              << std::endl;

    auto response_packet = network::CreateLoginResponsePacket<PacketType>(500);  // Code 500: Internal Server Error
    connection->Send(std::move(response_packet));
  }
}

}  // namespace rtype

#endif  // USER_LOGIN_HANDLER_HPP_
