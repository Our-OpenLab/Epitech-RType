

#ifndef CONCURRENT_QUEUE_HPP_
#define CONCURRENT_QUEUE_HPP_

#include <mutex>
#include <queue>
#include <optional>

namespace network {

template <typename T>
class ConcurrentQueue final {
 public:
  ConcurrentQueue() = default;
  ConcurrentQueue(const ConcurrentQueue&) = delete;
  ConcurrentQueue& operator=(const ConcurrentQueue&) = delete;

  ~ConcurrentQueue() = default;

  /*
  std::optional<std::reference_wrapper<const T>> front() const {
      std::lock_guard lock(mutex_);
      if (queue_.empty()) {
          return std::nullopt;
      }
      return std::cref(queue_.front());
  }
  */

  const T& front() {
    std::lock_guard lock(mutex_);
    return queue_.front();
  }

  unsigned long size() const {
    std::lock_guard lock(mutex_);
    return queue_.size();
  }

  std::optional<T> pop() {
    std::lock_guard lock(mutex_);
    if (queue_.empty()) {
      return std::nullopt;
    }
    T tmp = std::move(queue_.front());
    queue_.pop();
    return tmp;
  }

  bool try_pop(T& value) {
    std::lock_guard lock(mutex_);
    if (queue_.empty()) {
      return false;
    }
    value = std::move(queue_.front());
    queue_.pop();
    return true;
  }

  void push(const T& item) {
    std::lock_guard lock(mutex_);
    queue_.push(item);
  }

  void push(T&& item) {
    std::lock_guard lock(mutex_);
    queue_.push(std::move(item));
  }

  void clear() {
    std::lock_guard lock(mutex_);
    queue_.clear();
  }

  [[nodiscard]] bool empty() const {
    std::lock_guard lock(mutex_);
    return queue_.empty();
  }

 private:
  std::queue<T> queue_{};
  mutable std::mutex mutex_;
};

}  // namespace network

#endif  // CONCURRENT_QUEUE_HPP_
