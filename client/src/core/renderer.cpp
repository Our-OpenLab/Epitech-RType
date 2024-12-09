#include <stdexcept>

#include "client/core/renderer.hpp"

Renderer::Renderer(const std::string& title) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    throw std::runtime_error("Failed to initialize SDL: " + std::string(SDL_GetError()));
  }

  SDL_DisplayMode display_mode;
  if (SDL_GetCurrentDisplayMode(0, &display_mode) != 0) {
    throw std::runtime_error("Failed to get display mode: " +
                             std::string(SDL_GetError()));
  }

  const int screen_width = display_mode.w;
  const int screen_height = display_mode.h;

  window_ = SDL_CreateWindow(
      title.c_str(),
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      screen_width,
      screen_height,
      SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN
  );

  if (!window_) {
    throw std::runtime_error("Failed to create SDL window: " + std::string(SDL_GetError()));
  }

  renderer_ = SDL_CreateRenderer(
      window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  if (!renderer_) {
    throw std::runtime_error("Failed to create SDL renderer: " + std::string(SDL_GetError()));
  }

  std::cout << "Renderer initialized in fullscreen mode at resolution: "
            << screen_width << "x" << screen_height << std::endl;
}


Renderer::~Renderer() {
  Shutdown();
}

void Renderer::DrawGame(const client::GameState& game_state) const {
  auto& registry = game_state.GetRegistry();
  auto& positions = registry.get_components<Position>();
  auto& players = registry.get_components<Player>();
  auto& projectiles = registry.get_components<Projectile>();

  for (size_t i = 0; i < positions.size(); ++i) {
    if (!positions[i].has_value()) {
      continue;
    }

    const auto& [x, y] = *positions[i];

    SDL_Rect rect;

    if (i < players.size() && players[i].has_value()) {
      rect = SDL_Rect{
        static_cast<int>(x),
        static_cast<int>(y),
        50,
        50
    };

      SDL_SetRenderDrawColor(renderer_, 0, 0, 255, 255);
    }

    else if (i < projectiles.size() && projectiles[i].has_value()) {
      rect = SDL_Rect{
        static_cast<int>(x),
        static_cast<int>(y),
        20,
        10
    };

      SDL_SetRenderDrawColor(renderer_, 255, 0, 0, 255);
    } else {
      continue;
    }

    SDL_RenderFillRect(renderer_, &rect);
  }
}

void Renderer::Clear() const {
  SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
  SDL_RenderClear(renderer_);
}

void Renderer::Present() const {
  SDL_RenderPresent(renderer_);
}

void Renderer::Shutdown() {
  if (renderer_) {
    SDL_DestroyRenderer(renderer_);
    renderer_ = nullptr;
  }
  if (window_) {
    SDL_DestroyWindow(window_);
    window_ = nullptr;
  }
}
