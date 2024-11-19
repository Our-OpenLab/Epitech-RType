

#ifndef CONCURRENT_QUEUE_HPP_
#define CONCURRENT_QUEUE_HPP_

#include <mutex>
#include <queue>

namespace network {

    template<typename T>
    class ConcurentQueue final
    {
      public:

        ConcurentQueue() = default;
        ConcurentQueue(const ConcurentQueue&) = delete;
        ConcurentQueue& operator=(const ConcurentQueue&) = delete;

        ~ConcurentQueue() = default;

        unsigned long size() const {
            std::lock_guard lock(mutex_);
            return queue_.size();
        }

        std::optional<T> pop() {
            std::lock_guard lock(mutex_);
            if (queue_.empty()) {
                return std::nullopt;
            }
            T tmp = queue_.front();
            queue_.pop();
            return tmp;
        }

        bool try_pop(T& value) {
            std::lock_guard lock(mutex_);
            if (queue_.empty()) {
                return false;
            }
            value = queue_.front();
            queue_.pop();
            return true;
        }

        void push(const T &item) {
            std::lock_guard lock(mutex_);
            queue_.push(item);
        }

        void clear()
        {
            std::lock_guard lock(mutex_);
            queue_.clear();
        }

      private:
        std::queue<T> queue_{};
        mutable std::mutex mutex_;

        [[nodiscard]] bool empty() const {
            return queue_.empty();
        }
    };

}

#endif // CONCURRENT_QUEUE_HPP_
