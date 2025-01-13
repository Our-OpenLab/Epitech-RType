#ifndef RTYPE_CLIENT_CORE_MESSAGE_DISPATCHER_TPP
#define RTYPE_CLIENT_CORE_MESSAGE_DISPATCHER_TPP

namespace network {

template <typename PacketType>
MessageDispatcher<PacketType>::MessageDispatcher(rtype::EventQueue& event_queue)
    : event_queue_(event_queue) {
  InitializeHandlers();
}

template <typename PacketType>
void MessageDispatcher<PacketType>::Dispatch(Packet<PacketType>&& packet) const {
  if (const auto index = static_cast<size_t>(packet.header.type); index < handlers_.size() && handlers_[index]) {
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
    std::cerr << "[MessageDispatcher][ERROR] Invalid packet type index: " << index << "\n";
  }
}

template <typename PacketType>
void MessageDispatcher<PacketType>::InitializeHandlers() {
  handlers_.fill(nullptr);

  // Example: Registering a Ping handler
  //RegisterHandler(PacketType::Ping, [this](Packet<PacketType>&& packet) {
  //  event_queue_.Publish(rtype::EventType::kPing, std::make_shared<Packet<PacketType>>(std::move(packet)));
  //});
}

template <typename PacketType>
void MessageDispatcher<PacketType>::DefaultHandler(Packet<PacketType>&& packet) const {
  std::cerr << "[MessageDispatcher][WARNING] Unhandled packet type: "
            << static_cast<size_t>(packet.header.type) << "\n";
}

}  // namespace network

#endif  // RTYPE_CLIENT_CORE_MESSAGE_DISPATCHER_TPP
