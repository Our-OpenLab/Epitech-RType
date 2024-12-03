#include <iostream>
#include <stdexcept>
#include <string>

#include <client/core/client.hpp>

int main(const int argc, const char* argv[]) {
  try {
    std::string host = "localhost";
    std::string port = "4242";

    if (argc >= 3) {
      host = argv[1];
      port = argv[2];
    } else {
      std::cout << "[INFO] Using default host and port: localhost:12345\n";
    }

    Client client(host, port);

    client.run();

    return 0;
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return -1;
  }
}
