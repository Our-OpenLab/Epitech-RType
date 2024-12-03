#include <stdexcept>

#include <client/core/renderer.hpp>

Renderer::Renderer(const int width, const int height, const std::string& title) {
  window_ = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
  if (!window_) {
    throw std::runtime_error("Failed to create SDL window: " + std::string(SDL_GetError()));
  }

  renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!renderer_) {
    throw std::runtime_error("Failed to create SDL renderer: " + std::string(SDL_GetError()));
  }
}

Renderer::~Renderer() {
  shutdown();
}

void Renderer::draw_game(const GameState& game_state) const {
  for (const auto& [id, player] : game_state.get_players()) {

    SDL_Rect rect{static_cast<int>(player.x), static_cast<int>(player.y), 50, 50};
    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer_, &rect);
  }
}

void Renderer::clear() const {
  SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
  SDL_RenderClear(renderer_);
}

void Renderer::present() const {
  SDL_RenderPresent(renderer_);
}

void Renderer::shutdown() {
  if (renderer_) {
    SDL_DestroyRenderer(renderer_);
    renderer_ = nullptr;
  }

  if (window_) {
    SDL_DestroyWindow(window_);
    window_ = nullptr;
  }
}
