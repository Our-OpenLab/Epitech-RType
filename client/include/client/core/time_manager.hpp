#ifndef TIME_MANAGER_HPP_
#define TIME_MANAGER_HPP_

#include <chrono>
#include <iostream>

class TimeManager {
public:
  using Clock = std::chrono::steady_clock;
  using TimePoint = Clock::time_point;
  using Duration = Clock::duration;

  void update() {
    const auto now = Clock::now();
    delta_time_ = now - current_time_;
    current_time_ = now;
  }

  [[nodiscard]] TimePoint now() const {
    return current_time_;
  }

  [[nodiscard]] Duration delta_time() const {
    return delta_time_;
  }

  void start_tick(const Duration& tick_duration) {
    tick_duration_ = tick_duration;
    next_tick_time_ = current_time_ + tick_duration_;
  }

  void wait_for_next_tick() {
    if (next_tick_time_ > current_time_) {
      std::this_thread::sleep_for(next_tick_time_ - current_time_);
    } else {
      std::cerr << "[TimeManager] Tick overrun by "
                << std::chrono::duration_cast<std::chrono::milliseconds>(current_time_ - next_tick_time_).count()
                << " ms\n";
    }

    next_tick_time_ += tick_duration_;
  }

  [[nodiscard]] Duration time_since(const TimePoint& point) const {
    return current_time_ - point;
  }

private:
  TimePoint current_time_ = Clock::now();
  Duration delta_time_ = Duration::zero();
  Duration tick_duration_ = Duration::zero();
  TimePoint next_tick_time_ = current_time_;
};

#endif // TIME_MANAGER_HPP_

