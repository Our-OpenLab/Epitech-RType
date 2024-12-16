#ifndef EVENT_QUEUE_HPP_
#define EVENT_QUEUE_HPP_

#include <queue>
#include <mutex>
#include <functional>

class EventQueue {
public:
  void Push(std::function<void()>&& event) {
    std::lock_guard<std::mutex> lock(mutex_);
    events_.emplace(std::move(event));
  }

  void Process() {
    std::queue<std::function<void()>> local_queue;

    {
      std::lock_guard<std::mutex> lock(mutex_);
      std::swap(local_queue, events_);
    }

    while (!local_queue.empty()) {
      local_queue.front()();
      local_queue.pop();
    }
  }

private:
  std::queue<std::function<void()>> events_;
  std::mutex mutex_;
};

#endif // EVENT_QUEUE_HPP_
