#ifndef SIGNAL_HANDLER_HPP_
#define SIGNAL_HANDLER_HPP_

#include <csignal>
#include <atomic>

class SignalHandler {
public:
  static void Initialize() {
    std::signal(SIGINT, HandleSignal);
  }

  static bool IsStopRequested() {
    return stop_requested_;
  }

private:
  static void HandleSignal(int) {
    stop_requested_ = true;
  }

  static inline std::atomic<bool> stop_requested_{false};
};

#endif // SIGNAL_HANDLER_HPP_
