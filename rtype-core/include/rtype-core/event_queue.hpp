#ifndef EVENT_QUEUE_HPP_
#define EVENT_QUEUE_HPP_

#include <array>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>

#include <concurrentqueue.h>

#include "event_type.hpp"

namespace rtype {

/**
 * @brief A thread-safe event queue for decoupling systems through asynchronous events.
 *
 * The EventQueue allows systems to subscribe to specific event types with handlers
 * and publish events to be processed asynchronously. It uses a separate queue
 * for each event type to maximize efficiency and avoid contention between event types.
 */
class EventQueue {
public:
  using EventHandler = std::function<void(std::shared_ptr<void>)>;

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
   * by the ProcessEvents method.
   *
   * @tparam Event The type of the event payload (e.g., a struct or class).
   * @param event_type The type of the event being published.
   * @param event A shared pointer to the event data.
   */
  template <typename Event>
  void Publish(EventType event_type, std::shared_ptr<Event> event) {
    queues_[static_cast<size_t>(event_type)].enqueue(std::move(event));
  }

  /**
   * @brief Processes all pending events for all event types.
   *
   * This method dequeues events from all internal queues and invokes
   * the subscribed handlers for each event. Should be called regularly
   * in the main game loop or a designated processing thread.
   */
  void ProcessEvents() {
    for (size_t i = 0; i < queues_.size(); ++i) {
      auto& queue = queues_[i];
      std::shared_ptr<void> event;

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

private:
  /**
   * @brief A separate queue for each event type to minimize contention.
   */
  std::array<moodycamel::ConcurrentQueue<std::shared_ptr<void>>, static_cast<size_t>(EventType::MaxTypes)> queues_;

  /**
   * @brief A list of handlers for each event type.
   *
   * Each event type has its own vector of handlers, which are invoked
   * when an event of that type is processed.
   */
  std::array<std::vector<EventHandler>, static_cast<size_t>(EventType::MaxTypes)> handlers_;
};

}  // namespace rtype

#endif  // EVENT_QUEUE_HPP_
