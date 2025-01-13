#ifndef RTYPE_CLIENT_CORE_EVENT_QUEUE_HPP_
#define RTYPE_CLIENT_CORE_EVENT_QUEUE_HPP_

#include <array>
#include <functional>
#include <iostream>
#include <vector>

#include <concurrentqueue.h>

#include "event_type.hpp"

namespace rtype {

/**
 * @brief A thread-safe, generic event queue for decoupling systems through asynchronous events.
 *
 * The `EventQueue` allows systems to publish and process events of a specific type (`Event`).
 * Handlers can subscribe to specific event types and are called when events are processed.
 *
 * @tparam Event The type of the events managed by the queue (e.g., packets, custom data structures).
 */
template <typename Event>
class EventQueue {
public:
  /**
   * @brief Defines the signature of an event handler.
   *
   * Each handler takes a constant reference to an event of type `Event`.
   */
  using EventHandler = std::function<void(const Event&)>;

  /**
   * @brief Subscribes a handler to a specific event type.
   *
   * Handlers are invoked whenever an event of the corresponding type is published.
   *
   * @param event_type The type of the event to subscribe to.
   * @param handler A callable object (lambda, function, etc.) to handle the event.
   */
  void Subscribe(EventType event_type, EventHandler handler) {
    handlers_[static_cast<size_t>(event_type)].push_back(std::move(handler));
  }

  /**
   * @brief Publishes an event to the queue for the specified event type.
   *
   * The event will be stored in the appropriate queue and processed later
   * by the `ProcessEvents` method.
   *
   * @param event_type The type of the event being published.
   * @param event The event data to publish.
   */
  void Publish(EventType event_type, Event event) {
    queues_[static_cast<size_t>(event_type)].enqueue(std::move(event));
  }

  /**
   * @brief Processes all pending events for all event types.
   *
   * This method dequeues events from all internal queues and invokes
   * the subscribed handlers for each event.
   *
   * Should be called regularly in the main game loop or a designated processing thread.
   */
  void ProcessEvents() {
    for (size_t i = 0; i < queues_.size(); ++i) {
      auto& queue = queues_[i];
      Event event;

      // Process all events for the current event type
      while (queue.try_dequeue(event)) {
        for (auto& handler : handlers_[i]) {
          try {
            handler(event);
          } catch (const std::exception& e) {
            std::cerr << "[EventQueue][ERROR] Exception in handler: " << e.what() << std::endl;
          }
        }
      }
    }
  }

  /**
   * @brief Clears all handlers for a specific event type.
   *
   * Removes all handlers subscribed to the specified `event_type`.
   *
   * @param event_type The type of the event to clear handlers for.
   */
  void ClearHandlers(EventType event_type) {
    handlers_[static_cast<size_t>(event_type)].clear();
  }

private:
  /**
   * @brief A separate queue for each event type to minimize contention.
   *
   * Each event type has its own concurrent queue.
   */
  std::array<moodycamel::ConcurrentQueue<Event>, static_cast<size_t>(EventType::MaxTypes)> queues_;

  /**
   * @brief A list of handlers for each event type.
   *
   * Each event type has its own vector of handlers, which are invoked
   * when an event of that type is processed.
   */
  std::array<std::vector<EventHandler>, static_cast<size_t>(EventType::MaxTypes)> handlers_;
};

}  // namespace rtype

#endif  // RTYPE_CLIENT_CORE_EVENT_QUEUE_HPP_
