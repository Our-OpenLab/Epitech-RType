#ifndef CONTROLLER_HPP_
#define CONTROLLER_HPP_

#include <atomic>
#include <thread>
#include <chrono>
#include <iostream>

#include "signal_handler.hpp"

template <typename ServerType>
class Controller {
public:
  explicit Controller(ServerType& server)
      : server_(server) {}

  void Start() {
    SignalHandler::Initialize();

    if (!server_.Start()) {
      std::cerr << "[Controller][ERROR] Failed to start the server.\n";
      return;
    }

    std::cout << "[Controller] Server started successfully.\n";

    while (is_running_) {
      if (SignalHandler::IsStopRequested()) {
        Stop();
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  void Stop() {
    if (!is_running_) return;

    is_running_ = false;
    server_.Stop();
    std::cout << "[Controller] Server shutdown completed.\n";
  }

private:
  ServerType& server_;
  std::atomic<bool> is_running_{true};
};

#endif // CONTROLLER_HPP_
