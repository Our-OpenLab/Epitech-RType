#ifndef PING_MANAGER_HPP_
#define PING_MANAGER_HPP_

#include <chrono>
#include <iostream>

class Client;

class PingManager {
public:
  explicit PingManager(Client& client)
      : client_(client), last_ping_time_(std::chrono::steady_clock::now()), current_ping_(-1) {}

  void update();

  void set_ping(const int ping) {
    if (ping >= 0) {
      current_ping_ = ping;
    } else {
      std::cerr << "[PingManager][ERROR] Invalid ping value: " << ping << std::endl;
    }
  }

  [[nodiscard]] int get_ping() const {
    return current_ping_;
  }

private:
  void send_ping();

  Client& client_;
  std::chrono::steady_clock::time_point last_ping_time_;
  int current_ping_;
};

#endif // PING_MANAGER_HPP_
