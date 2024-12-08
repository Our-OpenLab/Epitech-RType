#include <stdexcept>

#include "client/core/renderer.hpp"

Renderer::Renderer(int width, int height, const std::string& title) {
  window_ = SDL_CreateWindow(
      title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      width, height, SDL_WINDOW_SHOWN);

  if (!window_) {
    throw std::runtime_error("Failed to create SDL window: " + std::string(SDL_GetError()));
  }

  renderer_ = SDL_CreateRenderer(
      window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  if (!renderer_) {
    throw std::runtime_error("Failed to create SDL renderer: " + std::string(SDL_GetError()));
  }
}

Renderer::~Renderer() {
  Shutdown();
}

void Renderer::DrawGame(client::GameState& game_state) const {
  auto& registry = game_state.GetRegistry();
  auto& positions = registry.get_components<Position>();

  for (size_t i = 0; i < positions.size(); ++i) {
    if (!positions[i].has_value()) {
      continue;
    }

    const auto& [x, y] = *positions[i];

    SDL_Rect rect{
      static_cast<int>(x),
      static_cast<int>(y),
      50,
      50
  };

    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer_, &rect);
  }
}

void Renderer::Clear() const {
  SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);  // Black background
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
