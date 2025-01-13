#ifndef RTYPE_CLIENT_CORE_MESSAGE_DISPATCHER_TPP_
#define RTYPE_CLIENT_CORE_MESSAGE_DISPATCHER_TPP_

#include <iostream>

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

  // Example: Registering a handler for LoginResponse packets
  RegisterHandler(PacketType::kUserLoginResponse, [this](Packet<PacketType>&& packet) {
    event_queue_.Publish(
        rtype::EventType::LoginResponse,
        std::move(packet)  // Move the packet to the EventQueue
    );
  });

  RegisterHandler(PacketType::kUserRegisterResponse, [this](Packet<PacketType>&& packet) {
    event_queue_.Publish(
        rtype::EventType::RegisterResponse,
        std::move(packet)  // Move the packet to the EventQueue
    );
  });
}

}  // namespace network

#endif  // RTYPE_CLIENT_CORE_MESSAGE_DISPATCHER_TPP_
