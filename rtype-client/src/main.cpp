#include "core/main_server.hpp"
#include "core/controller.hpp"

#include <core/renderer.hpp>

#include "scenes/login_scene.hpp"

#include "ui/ui_element.hpp"
#include "ui/button.hpp"
#include "ui/text.hpp"
#include "ui/text_box.hpp"

#include "core/resource_manager.hpp"

int main(const int argc, const char* argv[]) {
  try {
    if (argc < 5) {
      std::cerr << "[ERROR] Usage: " << argv[0]
                << " <host> <tcp_port> <udp_port> <local_ip>\n";
      std::cerr << "[INFO] Please provide the server details and your local IP address.\n";
      return -1;
    }

    // Récupérer les arguments
    std::string host = argv[1];           // Adresse IP ou DNS du serveur
    std::string tcp_port = argv[2];       // Port TCP fourni par l'utilisateur
    uint16_t udp_port = static_cast<uint16_t>(std::stoi(argv[3])); // Port UDP fourni par l'utilisateur
    std::string local_ip = argv[4];       // Adresse IP locale fournie par l'utilisateur

    std::cout << "[INFO] Using local IP: " << local_ip << "\n";
    std::cout << "[INFO] Connecting to server at " << host
              << " on TCP port " << tcp_port
              << " and UDP port " << udp_port << "\n";

    if (rtype::MainServer<network::MyPacketType> client(local_ip);
        !client.Start(host, tcp_port, udp_port)) {
      std::cerr << "[main] Failed to start client.\n";
      return 1;
        }

    std::cout << "[main] Client finished.\n";
    return 0;

  } catch (const std::exception& e) {
    std::cerr << "[ERROR] " << e.what() << std::endl;
    return -1;
  }
}
