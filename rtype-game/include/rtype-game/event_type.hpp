#ifndef EVENT_TYPE_HPP_
#define EVENT_TYPE_HPP_

#include <cstddef>

namespace rtype {

/**
 * @brief Enum class defining the different types of events supported.
 */
enum class EventType : size_t {
  PingTCP,
  PingUDP,
  ClientAccepted,
  UdpPortTCP,
  PlayerInputUDP,
  UnhandledTCP,
  UnhandledUDP,
  MaxTypes  // Must always be the last entry, representing the number of event types.
};

}

#endif  // EVENT_TYPE_HPP_
