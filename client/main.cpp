#include <iostream>
#include <network/client.hpp>
#include <network/client_connection.hpp>
#include <network/concurrent_queue.hpp>
#include <network/connection.hpp>
#include <network/protocol.hpp>
#include <network/server_connection.hpp>
#include <SDL2/SDL.h>
#include <network/protocol.hpp>


int main() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
    std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
    return 1;
  }

  SDL_Window* window = SDL_CreateWindow(
      "Network Client",
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      640,
      480,
      SDL_WINDOW_SHOWN
  );

  if (!window) {
    std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
    SDL_Quit();
    return 1;
  }

  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer) {
    std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  network::Client client;

  if (!client.connect("127.0.0.1", "4242")) {
    std::cerr << "Failed to connect to the server!" << std::endl;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  bool quit = false;
  bool key[3] = { false, false, false };
  bool old_key[3] = { false, false, false };

  std::cout << "Client connected. Use keys 1, 2, and 3 for actions.\n";

  while (!quit) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        quit = true;
      }

      if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        const bool is_pressed = event.type == SDL_KEYDOWN;

        switch (event.key.keysym.sym) {
          case SDLK_1:
            key[0] = is_pressed;
          break;
          case SDLK_2:
            key[1] = is_pressed;
          break;
          case SDLK_3:
            key[2] = is_pressed;
          break;
          default:;
        }
      }
    }
    if (key[0] && !old_key[0]) {
      std::cout << "[Client][INFO] Sending Ping to server." << std::endl;

      network::Packet packet;
      packet.header.type = network::PacketType::Ping;

      const auto timestamp = static_cast<std::uint32_t>(
         std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
             .count());

      packet.push(timestamp);
      client.send(packet);
    }
   // if (key[1] && !old_key[1]) {
   //   std::cout << "[Client][INFO] Sending Message to all clients." << std::endl;

   //   network::Packet packet;
   //   packet.header.type = network::PacketType::MessageAll;
   //   client.send(packet);
   // }
    if (key[2] && !old_key[2]) {
      std::cout << "[Client][INFO] Exiting." << std::endl;
      quit = true;
    }

    for (int i = 0; i < 3; ++i) {
      old_key[i] = key[i];
    }

    if (client.is_connected()) {
      while (!client.get_received_queue().empty()) {
        auto message = client.get_received_queue().pop();

        switch (message->header.type)
        {
          case network::PacketType::ServerAccept:
          {
            // Server has responded to a ping request
            std::cout << "Server Accepted Connection\n";
          }
          break;

          case network::PacketType::Pong:
          {
            try {
              // Extraire le timestamp envoyé par le serveur
              if (message->body.size() == sizeof(std::uint32_t)) {
                const auto server_timestamp = message->extract<std::uint32_t>();

                // Obtenir le timestamp actuel
                const auto current_timestamp = static_cast<std::uint32_t>(
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch()).count()
                );

                // Calculer le Round Trip Time (RTT)
                const auto rtt = current_timestamp - server_timestamp;

                std::cout << "[Client][INFO] Pong received. RTT: " << rtt << " ms\n";
              } else {
                std::cerr << "[Client][ERROR] Pong packet has incorrect size.\n";
              }
            } catch (const std::exception& e) {
              std::cerr << "[Client][ERROR] Exception handling Pong: " << e.what() << "\n";
            }
            break;
          }
/*
          case network::PacketType::ServerMessage:
          {
            // Server has responded to a message
            const auto clientID = message->extract<uint32_t>();
            std::cout << "Hello from [" << clientID << "]\n";
          } break;
          */
          default:
            break;
        }
      }
    } else {
      std::cout << "[Client][ERROR] Server Down." << std::endl;
      quit = true;
    }

    SDL_Delay(16);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
