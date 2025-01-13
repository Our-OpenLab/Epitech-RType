#include "core/main_server.hpp"
#include "core/controller.hpp"

#include <core/renderer.hpp>

#include "scenes/login_scene.hpp"

#include "ui/ui_element.hpp"
#include "ui/button.hpp"
#include "ui/text.hpp"
#include "ui/text_box.hpp"

int main(const int argc, const char* argv[]) {
  try {
    std::string host = "localhost";
    std::string tcp_port = "4242";
    uint16_t udp_port = 4243;

    if (argc >= 4) {
      host = argv[1];
      tcp_port = argv[2];
      udp_port = static_cast<uint16_t>(std::stoi(argv[3]));
    } else if (argc == 3) {
      host = argv[1];
      tcp_port = argv[2];
      std::cout << "[INFO] Using default UDP port: 4243\n";
    } else {
      std::cout << "[INFO] Using default host, TCP port, and UDP port: localhost:4242 (UDP: 4243)\n";
    }

    rtype::MainServer<network::MyPacketType> client;

    Controller controller(client);

    controller.Start(host, tcp_port, udp_port);

  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return -1;
  }
}
