#include <iostream>
#include <cstdlib>

#include "rtype-game/controller.hpp"
#include "rtype-game/main_server.hpp"
#include "rtype-game/my_packet_types.hpp"

int main(const int argc, const char* argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <TCP_PORT> <UDP_PORT>" << std::endl;
    return EXIT_FAILURE;
  }

  const int tcp_port = std::atoi(argv[1]);
  const int udp_port = std::atoi(argv[2]);

  if (tcp_port <= 0 || udp_port <= 0) {
    std::cerr << "Error: Invalid port numbers provided." << std::endl;
    return EXIT_FAILURE;
  }

  try {
    rtype::MainServer<network::MyPacketType> game_server(tcp_port, udp_port);
    Controller controller(game_server);

    controller.Start();
  } catch (const std::exception& e) {
    std::cerr << "An error occurred: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
