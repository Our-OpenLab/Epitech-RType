#ifndef RTYPE_CLIENT_CORE_MESSAGE_DISPATCHER_TPP_
#define RTYPE_CLIENT_CORE_MESSAGE_DISPATCHER_TPP_

#include <iostream>

#include "event_type.hpp"

namespace network {

template <typename PacketType>
MessageDispatcher<PacketType>::MessageDispatcher(
    rtype::EventQueue<Packet<PacketType>>& event_queue)
    : event_queue_(event_queue) {
  InitializeHandlers();
}

template <typename PacketType>
void MessageDispatcher<PacketType>::Dispatch(Packet<PacketType>&& packet) const {
  const auto index = static_cast<size_t>(packet.header.type);
  if (index < handlers_.size() && handlers_[index]) {
    handlers_[index](std::move(packet));
  } else {
    DefaultHandler(std::move(packet));
  }
}

template <typename PacketType>
void MessageDispatcher<PacketType>::RegisterHandler(PacketType packet_type, Handler handler) {
  const auto index = static_cast<size_t>(packet_type);
  if (index < handlers_.size()) {
    handlers_[index] = std::move(handler);
  } else {
    std::cerr << "[MessageDispatcher][ERROR] Invalid packet type index: " << index << std::endl;
  }
}

template <typename PacketType>
void MessageDispatcher<PacketType>::DefaultHandler(Packet<PacketType>&& packet) const {
  std::cerr << "[MessageDispatcher][WARNING] Unhandled packet type: "
            << static_cast<size_t>(packet.header.type) << std::endl;
}

template <typename PacketType>
void MessageDispatcher<PacketType>::InitializeHandlers() {
  handlers_.fill(nullptr);

  handlers_[static_cast<size_t>(PacketType::kUserLoginResponse)] =
    [this](Packet<PacketType>&& packet) {
      event_queue_.Publish(rtype::EventType::LoginResponse, std::move(packet));
  };

  handlers_[static_cast<size_t>(PacketType::kUserRegisterResponse)] =
    [this](Packet<PacketType>&& packet) {
      event_queue_.Publish(rtype::EventType::RegisterResponse, std::move(packet));
  };

  handlers_[static_cast<size_t>(PacketType::kGetUserListResponse)] =
    [this](Packet<PacketType>&& packet) {
      event_queue_.Publish(rtype::EventType::GetUserListResponse, std::move(packet));
  };

  handlers_[static_cast<size_t>(PacketType::kPrivateChatHistoryResponse)] =
    [this](Packet<PacketType>&& packet) {
      event_queue_.Publish(rtype::EventType::PrivateChatHistoryResponse, std::move(packet));
  };

  handlers_[static_cast<size_t>(PacketType::kPrivateChatMessage)] =
    [this](Packet<PacketType>&& packet) {
      event_queue_.Publish(rtype::EventType::PrivateChatMessage, std::move(packet));
    };

  handlers_[static_cast<size_t>(PacketType::kCreateLobbyResponse)] =
    [this](Packet<PacketType>&& packet) {
      event_queue_.Publish(rtype::EventType::CreateLobbyResponse, std::move(packet));
  };

  handlers_[static_cast<size_t>(PacketType::kGetLobbyPlayersResponse)] =
    [this](Packet<PacketType>&& packet) {
      event_queue_.Publish(rtype::EventType::GetLobbyPlayersResponse, std::move(packet));
    };

  handlers_[static_cast<size_t>(PacketType::kLeaveLobbyResponse)] =
    [this](Packet<PacketType>&& packet) {
      event_queue_.Publish(rtype::EventType::LeaveLobbyResponse, std::move(packet));
    };

  handlers_[static_cast<size_t>(PacketType::kPlayerJoinedLobby)] =
    [this](Packet<PacketType>&& packet) {
      event_queue_.Publish(rtype::EventType::PlayerJoinedLobby, std::move(packet));
    };

  handlers_[static_cast<size_t>(PacketType::kPlayerLeftLobby)] =
    [this](Packet<PacketType>&& packet) {
      event_queue_.Publish(rtype::EventType::PlayerLeftLobby, std::move(packet));
    };

  handlers_[static_cast<size_t>(PacketType::kJoinLobbyResponse)] =
    [this](Packet<PacketType>&& packet) {
      event_queue_.Publish(rtype::EventType::JoinLobbyResponse, std::move(packet));
    };

  handlers_[static_cast<size_t>(PacketType::kGetLobbyListResponse)] =
    [this](Packet<PacketType>&& packet) {
      event_queue_.Publish(rtype::EventType::GetLobbyListResponse, std::move(packet));
    };

  handlers_[static_cast<size_t>(PacketType::kPlayerReadyResponse)] =
    [this](Packet<PacketType>&& packet) {
      event_queue_.Publish(rtype::EventType::PlayerReadyResponse, std::move(packet));
    };

  handlers_[static_cast<size_t>(PacketType::kLobbyPlayerReady)] =
    [this](Packet<PacketType>&& packet) {
      event_queue_.Publish(rtype::EventType::LobbyPlayerReady, std::move(packet));
    };

  handlers_[static_cast<size_t>(PacketType::kGameConnectionInfo)] =
    [this](Packet<PacketType>&& packet) {
      event_queue_.Publish(rtype::EventType::GameConnectionInfo, std::move(packet));
    };

  handlers_[static_cast<size_t>(PacketType::kPong)] =
    [this](Packet<PacketType>&& packet) {
      event_queue_.Publish(rtype::EventType::Pong, std::move(packet));
    };

  handlers_[static_cast<size_t>(PacketType::kPlayerAssign)] = [this](const Packet<PacketType>&& packet) {
    event_queue_.Publish(rtype::EventType::PlayerAssign, std::move(packet));
  };

  handlers_[static_cast<size_t>(PacketType::kUpdatePlayers)] = [this](const Packet<PacketType>&& packet) {
    event_queue_.Publish(rtype::EventType::UpdatePlayers, std::move(packet));
  };

  handlers_[static_cast<size_t>(PacketType::kUpdateProjectiles)] = [this](const Packet<PacketType>&& packet) {
    event_queue_.Publish(rtype::EventType::UpdateProjectiles, std::move(packet));
  };

  handlers_[static_cast<size_t>(PacketType::kUpdateEnemies)] = [this](const Packet<PacketType>&& packet) {
    event_queue_.Publish(rtype::EventType::UpdateEnemies, std::move(packet));
  };

  handlers_[static_cast<size_t>(PacketType::kPlayerJoin)] = [this](const Packet<PacketType>&& packet) {
    event_queue_.Publish(rtype::EventType::PlayerJoined, std::move(packet));
  };

  handlers_[static_cast<size_t>(PacketType::kPlayerLeave)] = [this](const Packet<PacketType>&& packet) {
    event_queue_.Publish(rtype::EventType::PlayerLeave, std::move(packet));
  };

  handlers_[static_cast<size_t>(PacketType::kRemoveProjectile)] = [this](const Packet<PacketType>&& packet) {
    event_queue_.Publish(rtype::EventType::RemoveProjectile, std::move(packet));
  };

  handlers_[static_cast<size_t>(PacketType::kRemoveEnemy)] = [this](const Packet<PacketType>&& packet) {
    event_queue_.Publish(rtype::EventType::RemoveEnemy, std::move(packet));
  };

  handlers_[static_cast<size_t>(PacketType::kRemovePlayer)] = [this](const Packet<PacketType>&& packet) {
    event_queue_.Publish(rtype::EventType::RemovePlayer, std::move(packet));
  };
}

}  // namespace network

#endif  // RTYPE_CLIENT_CORE_MESSAGE_DISPATCHER_TPP_
