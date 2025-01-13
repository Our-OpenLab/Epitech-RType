#ifndef RTYPE_CLIENT_CORE_MESSAGE_DISPATCHER_HPP_
#define RTYPE_CLIENT_CORE_MESSAGE_DISPATCHER_HPP_

#include <array>
#include <functional>
#include <iostream>
#include <memory>
#include "event_queue.hpp"

namespace network {

/**
 * @brief Routes incoming packets to their respective handlers based on packet type.
 *
 * This class decouples packet handling from core networking logic and integrates
 * with an `EventQueue` to allow for asynchronous processing.
 *
 * @tparam PacketType Enum type representing different packet types.
 */
template <typename PacketType>
class MessageDispatcher {
 public:
  /**
   * @brief Function signature for handling packets.
   *
   * A handler takes a `Packet` object as an rvalue reference.
   */
  using Handler = std::function<void(Packet<PacketType>&&)>;

  /**
   * @brief Constructs a `MessageDispatcher` with a reference to an `EventQueue`.
   *
   * @param event_queue A reference to the `EventQueue` used for publishing events.
   */
  explicit MessageDispatcher(rtype::EventQueue<Packet<PacketType>>& event_queue);

  /**
   * @brief Dispatches a packet to the appropriate handler based on its type.
   *
   * If a handler for the packet type exists, it is invoked; otherwise, the default
   * handler is called.
   *
   * @param packet The packet to dispatch.
   */
  void Dispatch(Packet<PacketType>&& packet) const;

  /**
   * @brief Registers a handler for a specific packet type.
   *
   * @param packet_type The type of packet to handle.
   * @param handler The handler function to register.
   */
  void RegisterHandler(PacketType packet_type, Handler handler);

 private:
  /**
   * @brief Handles packets with no registered handler.
   *
   * Logs a warning about the unhandled packet.
   *
   * @param packet The unhandled packet.
   */
  void DefaultHandler(Packet<PacketType>&& packet) const;

  /**
   * @brief Initializes all handlers to `nullptr`.
   */
  void InitializeHandlers();

  rtype::EventQueue<Packet<PacketType>>& event_queue_;  ///< Reference to the EventQueue.
  std::array<Handler, static_cast<size_t>(PacketType::kMaxTypes)> handlers_;  ///< Array of handlers indexed by packet type.
};

}  // namespace network

#include "message_dispatcher.tpp"

#endif  // RTYPE_CLIENT_CORE_MESSAGE_DISPATCHER_HPP_
