#ifndef CONTROLLER_HPP_
#define CONTROLLER_HPP_

#include <asio.hpp>
#include <atomic>
#include <thread>
#include <iostream>
#include <string>

#include "signal_handler.hpp"

template <typename ServerType>
class Controller {
 public:
  explicit Controller(ServerType& server)
      : server_(server), input_stream_(io_context_, ::dup(STDIN_FILENO)) {}

  void Start(const std::string& host, const std::string& service, const uint16_t udp_port) {
    SignalHandler::Initialize();

    // 1) Start the server
    if (!server_.Start(host, service, udp_port)) {
      std::cerr << "[Controller][ERROR] Failed to start the server.\n";
      return;
    }
    std::cout << "[Controller] Server started successfully.\n";

    // 2) Start asynchronous input handling
    StartAsyncInput();

    // 3) Launch Asio IO context in a separate thread
    io_thread_ = std::thread([this]() { io_context_.run(); });

    // 4) Main control loop
    while (is_running_) {
      if (SignalHandler::IsStopRequested()) {
        Stop();
      }

      // Check if the server is still running
      if (!server_.IsRunning()) {
        std::cerr << "[Controller][INFO] Server stopped, shutting down controller.\n";
        Stop();
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // 5) Shutdown
    io_context_.stop();
    if (io_thread_.joinable()) {
      io_thread_.join();
    }

    std::cout << "[Controller] Server shutdown completed.\n";
  }

  void Stop() {
    if (!is_running_) {
      return;
    }
    is_running_ = false;

    server_.Stop();
  }

 private:
  void StartAsyncInput() {
    asio::async_read_until(
        input_stream_, buffer_, '\n',
        [this](const asio::error_code& ec, std::size_t length) {
          if (!ec && is_running_) {
            std::istream is(&buffer_);
            std::string command;
            std::getline(is, command);

            if (!command.empty()) {
              server_.HandleCommand(command);
            }

            // Clear the buffer and continue reading input
            buffer_.consume(length);
            StartAsyncInput();
          } else if (ec != asio::error::operation_aborted) {
            std::cerr << "[Controller][ERROR] Input error: " << ec.message() << "\n";
          }
        });
  }

  ServerType& server_;
  std::atomic<bool> is_running_{true};

  asio::io_context io_context_{};
  asio::posix::stream_descriptor input_stream_;
  asio::streambuf buffer_;
  std::thread io_thread_;
};

#endif  // CONTROLLER_HPP_
