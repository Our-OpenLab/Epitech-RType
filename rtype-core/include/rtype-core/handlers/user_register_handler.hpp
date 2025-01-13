#ifndef USER_REGISTER_HANDLER_HPP_
#define USER_REGISTER_HANDLER_HPP_

#include <iostream>
#include <memory>

#include "rtype-core/packet_factory.hpp"
#include "rtype-core/protocol.hpp"
#include "rtype-core/service_container.hpp"

namespace rtype {

template <typename PacketType>
void HandleUserRegister(
  const std::shared_ptr<void> raw_event,
    ServiceContainer& service_container)
{
    auto event = std::static_pointer_cast<
        std::pair<network::Packet<PacketType>,
                  std::shared_ptr<network::TcpServerConnection<PacketType>>>>(raw_event);

    const auto& packet = event->first;
    const auto& connection = event->second;

    if (packet.body.size() != sizeof(network::packets::RegisterPacket)) {
        std::cerr << "[UserRegisterHandler][ERROR] Invalid RegisterPacket size." << std::endl;

        auto response_packet = network::CreateRegisterResponsePacket<PacketType>(400);  // Failure
        connection->Send(std::move(response_packet));
        return;
    }

    const auto* register_data = reinterpret_cast<const network::packets::RegisterPacket*>(packet.body.data());
    const std::string username(register_data->username);
    const std::string password(register_data->password);

    if (const auto user_service = service_container.GetUserService()) {
        const bool success = user_service->RegisterUser(username, password);

        if (success) {
          std::cout << "[UserRegisterHandler] User registered successfully: " << username << std::endl;
        } else {
          std::cerr << "[UserRegisterHandler][ERROR] Failed to register user: " << username << std::endl;
        }

        auto response_packet = network::CreateRegisterResponsePacket<PacketType>(
            success ? 200 : 400);
        connection->Send(std::move(response_packet));
    } else {
        std::cerr << "[UserRegisterHandler][ERROR] UserService not available." << std::endl;

        auto response_packet = network::CreateRegisterResponsePacket<PacketType>(500);  // Server error
        connection->Send(std::move(response_packet));
    }
}

}  // namespace rtype

#endif  // USER_REGISTER_HANDLER_HPP_
