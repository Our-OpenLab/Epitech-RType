#include "rtype-core/controller.hpp"
#include "rtype-core/main_server.hpp"
#include "rtype-core/my_packet_types.hpp"


/**
 * @brief Reads the content of a file into a string.
 *
 * @param filepath Path to the file.
 * @return The content of the file as a string.
 * @throws std::runtime_error if the file cannot be opened.
 */
/*
std::string ReadPasswordFromFile(const std::string& filepath) {
  std::ifstream file(filepath);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + filepath);
  }

  std::string password;
  std::getline(file, password);  // Read the first line
  return password;
}
*/

int main()
{

  // Path provided by Kubernetes
  /*
  const std::string password_file = "/etc/secrets/postgres-password";
  const std::string password = ReadPasswordFromFile(password_file);

  // Use the password in your database connection string
  const std::string db_connection_string =
      "postgresql://postgres:" + password +
      "@my-postgres-postgresql.default.svc.cluster.local:5432/mydb";
      */

  rtype::MainServer<network::MyPacketType> game_server(4242, 4243, "postgresql://postgres:PiyYPnTAkE@my-postgres-postgresql.default.svc.cluster.local:5432/mydb");

 // rtype::MainServer<network::MyPacketType> game_server(4242, 4243, "postgresql://postgres:GFGsdqXILY@localhost:5432/mydb");

  Controller controller(game_server);

  controller.Start();

  return 0;
}
