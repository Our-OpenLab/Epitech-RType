#ifndef RTYPE_CLIENT_CORE_MESSAGE_DISPATCHER_HPP
#define RTYPE_CLIENT_CORE_MESSAGE_DISPATCHER_HPP

#include <array>
#include <functional>
#include <memory>
#include <network/tcp/tcp_server_connection.hpp>

#include "event_queue.hpp"

namespace network {

/**
 * @brief The MessageDispatcher class is responsible for routing incoming packets
 * to their appropriate handlers based on packet type.
 *
 * The class decouples packet handling logic from the core networking functionality,
 * allowing flexibility in handling various TCP and UDP packet types. It uses an
 * EventQueue to publish events for further processing.
 *
 * @tparam PacketType Enum type representing the different packet types.
 */
template <typename PacketType>
class MessageDispatcher {
public:
  /**
   * @brief Function signature for handling TCP/UDP packets.
   *
   * Each handler receives a Packet object, along with either a TCP connection
   * or a UDP endpoint, depending on the packet type.
   */
  using Handler = std::function<void(Packet<PacketType>&& packet)>;

  /**
   * @brief Constructs a MessageDispatcher with a reference to an EventQueue.
   *
   * @param event_queue Reference to the EventQueue used for publishing events.
   */
  explicit MessageDispatcher(rtype::EventQueue& event_queue);

  /**
   * @brief Dispatches a Packet to the corresponding handler based on its type.
   *
   * This function looks up a handler by examining the packet's type (stored in `packet.header.type`).
   * If a matching handler exists in `handlers_`, it is invoked; otherwise, `DefaultHandler()`
   * is called. The function takes ownership of the `packet` via rvalue reference and does not
   * modify the dispatcher’s internal state.
   *
   * @param packet The Packet object to be dispatched. This function accepts an rvalue reference,
   *               transferring ownership of the packet’s data.
   */
  void Dispatch(Packet<PacketType>&& packet) const;

  /**
   * @brief Registers a handler for a specific packet type.
   *
   * @param packet_type The type of the packet to handle.
   * @param handler The handler function to register.
   */
  void RegisterHandler(PacketType packet_type, Handler handler);

private:
  /**
   * @brief Default handler for unhandled packet types.
   * @param packet The unhandled packet.
   */
  void DefaultHandler(Packet<PacketType>&& packet) const;

  /**
   * @brief Initializes the handler array with default values.
   */
  void InitializeHandlers();

  rtype::EventQueue& event_queue_; ///< Reference to the EventQueue for publishing events.
  std::array<Handler, static_cast<size_t>(PacketType::kMaxTypes)> handlers_; ///< Array of handlers indexed by PacketType.
};

}  // namespace network

#include "message_dispatcher.tpp"

#endif  // RTYPE_CLIENT_CORE_MESSAGE_DISPATCHER_HPP
