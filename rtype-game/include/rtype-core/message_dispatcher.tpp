#ifndef MESSAGE_DISPATCHER_TPP_
#define MESSAGE_DISPATCHER_TPP_

#include "event_type.hpp"

namespace network {

template <typename PacketType>
MessageDispatcher<PacketType>::MessageDispatcher(rtype::EventQueue& event_queue)
    : event_queue_(event_queue) {
  InitializeHandlers();
}

template <typename PacketType>
void MessageDispatcher<PacketType>::Dispatch(OwnedPacket<PacketType>&& owned_packet) {
  std::visit(
      [this]<typename T0>(T0&& packet_variant) {
          this->HandlePacket(std::forward<T0>(packet_variant));
      },
      std::move(owned_packet)
  );
}

template <typename PacketType>
void MessageDispatcher<PacketType>::HandlePacket(OwnedPacketTCP<PacketType>&& packet_variant) const {
  auto& packet = packet_variant.packet;
  const auto& connection = packet_variant.connection;

  if (const auto index = static_cast<size_t>(packet.header.type);
      index < tcp_handlers_.size() && tcp_handlers_[index]) {
    tcp_handlers_[index](std::move(packet), connection);
  } else {
    DefaultTCPHandler(std::move(packet), connection);
  }
}

template <typename PacketType>
void MessageDispatcher<PacketType>::HandlePacket(OwnedPacketUDP<PacketType>&& packet_variant) const {
  auto& packet = packet_variant.packet;
  const auto& endpoint = packet_variant.endpoint;

  if (const auto index = static_cast<size_t>(packet.header.type);
      index < udp_handlers_.size() && udp_handlers_[index]) {
    udp_handlers_[index](std::move(packet), endpoint);
  } else {
    DefaultUDPHandler(std::move(packet), endpoint);
  }
}

template <typename PacketType>
void MessageDispatcher<PacketType>::DefaultTCPHandler(Packet<PacketType>&& packet,
                                                      const std::shared_ptr<TcpServerConnection<PacketType>>& connection) const {
  event_queue_.Publish(rtype::EventType::UnhandledTCP, std::make_shared<Packet<PacketType>>(std::move(packet)));
}

template <typename PacketType>
void MessageDispatcher<PacketType>::DefaultUDPHandler(Packet<PacketType>&& packet,
                                                      const asio::ip::udp::endpoint& endpoint) const {
  event_queue_.Publish(rtype::EventType::UnhandledUDP, std::make_shared<Packet<PacketType>>(std::move(packet)));
}

template <typename PacketType>
void MessageDispatcher<PacketType>::InitializeHandlers() {
  tcp_handlers_.fill(nullptr);
  udp_handlers_.fill(nullptr);

  tcp_handlers_[static_cast<size_t>(PacketType::kPing)] =
      [this](Packet<PacketType>&& packet, const std::shared_ptr<TcpServerConnection<PacketType>>& connection) {
        event_queue_.Publish(rtype::EventType::PingTCP, std::make_shared<Packet<PacketType>>(std::move(packet)));
  };

  udp_handlers_[static_cast<size_t>(PacketType::kPing)] =
      [this](Packet<PacketType>&& packet, const asio::ip::udp::endpoint& endpoint) {
        event_queue_.Publish(rtype::EventType::PingUDP, std::make_shared<Packet<PacketType>>(std::move(packet)));
  };

  tcp_handlers_[static_cast<size_t>(PacketType::kUserRegister)] =
    [this](Packet<PacketType>&& packet, const std::shared_ptr<TcpServerConnection<PacketType>>& connection) {
      using EventData = std::pair<Packet<PacketType>, std::shared_ptr<TcpServerConnection<PacketType>>>;

      event_queue_.Publish(rtype::EventType::UserRegister, std::make_shared<EventData>(std::move(packet), connection));
  };

  tcp_handlers_[static_cast<size_t>(PacketType::kUserLogin)] =
    [this](Packet<PacketType>&& packet, const std::shared_ptr<TcpServerConnection<PacketType>>& connection) {
      using EventData = std::pair<Packet<PacketType>, std::shared_ptr<TcpServerConnection<PacketType>>>;

      event_queue_.Publish(rtype::EventType::UserLogin, std::make_shared<EventData>(std::move(packet), connection));
  };

  tcp_handlers_[static_cast<size_t>(PacketType::kPrivateMessage)] =
    [this](Packet<PacketType>&& packet, const std::shared_ptr<TcpServerConnection<PacketType>>& connection) {
      using EventData = std::pair<Packet<PacketType>, std::shared_ptr<TcpServerConnection<PacketType>>>;

      event_queue_.Publish(rtype::EventType::PrivateMessage, std::make_shared<EventData>(std::move(packet), connection));
  };

  tcp_handlers_[static_cast<size_t>(PacketType::kCreateLobby)] =
    [this](Packet<PacketType>&& packet, const std::shared_ptr<TcpServerConnection<PacketType>>& connection) {
      using EventData = std::pair<Packet<PacketType>, std::shared_ptr<TcpServerConnection<PacketType>>>;

      event_queue_.Publish(rtype::EventType::CreateLobby, std::make_shared<EventData>(std::move(packet), connection));
  };

  tcp_handlers_[static_cast<size_t>(PacketType::kJoinLobby)] =
    [this](Packet<PacketType>&& packet, const std::shared_ptr<TcpServerConnection<PacketType>>& connection) {
      using EventData = std::pair<Packet<PacketType>, std::shared_ptr<TcpServerConnection<PacketType>>>;

      event_queue_.Publish(rtype::EventType::JoinLobby, std::make_shared<EventData>(std::move(packet), connection));
  };

  tcp_handlers_[static_cast<size_t>(PacketType::kPlayerReady)] =
    [this](Packet<PacketType>&& packet, const std::shared_ptr<TcpServerConnection<PacketType>>& connection) {
      using EventData = std::pair<Packet<PacketType>, std::shared_ptr<TcpServerConnection<PacketType>>>;

      event_queue_.Publish(rtype::EventType::PlayerReady, std::make_shared<EventData>(std::move(packet), connection));
  };

  tcp_handlers_[static_cast<size_t>(PacketType::kGetUserList)] =
    [this](Packet<PacketType>&& packet, const std::shared_ptr<TcpServerConnection<PacketType>>& connection) {
      using EventData = std::pair<Packet<PacketType>, std::shared_ptr<TcpServerConnection<PacketType>>>;

      event_queue_.Publish(rtype::EventType::GetUserList, std::make_shared<EventData>(std::move(packet), connection));
  };

  tcp_handlers_[static_cast<size_t>(PacketType::kPrivateChatHistory)] =
    [this](Packet<PacketType>&& packet, const std::shared_ptr<TcpServerConnection<PacketType>>& connection) {
      using EventData = std::pair<Packet<PacketType>, std::shared_ptr<TcpServerConnection<PacketType>>>;

      event_queue_.Publish(rtype::EventType::PrivateChatHistory, std::make_shared<EventData>(std::move(packet), connection));
  };
}

}

#endif  // MESSAGE_DISPATCHER_TPP_
