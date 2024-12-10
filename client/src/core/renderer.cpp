#include <stdexcept>

#include "client/core/renderer.hpp"

Renderer::Renderer(const int width, const int height, const std::string& title) {
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

  // Initialiser la caméra
  camera_ = {0.0f, 0.0f, width, height};
}

Renderer::~Renderer() {
  Shutdown();
}

void Renderer::UpdateCamera(const client::GameState& game_state) {
  auto& registry = game_state.GetRegistry();
  auto& players = registry.get_components<Player>();
  auto& positions = registry.get_components<Position>();

  for (size_t i = 0; i < players.size(); ++i) {
    if (players[i].has_value() && positions[i].has_value()) {
      const auto& [x, y] = *positions[i];
      camera_.x = x - camera_.width / 2.0f;
      camera_.y = y - camera_.height / 2.0f;
      return;
    }
  }
}


void Renderer::DrawArenaBoundaries() const {
  constexpr int arena_width = 2000;
  constexpr int arena_height = 2000;
  constexpr int border_thickness = 10;
  constexpr int neon_glow_offset = 5;

  // Décalage en fonction de la caméra
  int left = -camera_.x;
  int top = -camera_.y;

  // Dessiner les bordures
  SDL_SetRenderDrawColor(renderer_, 50, 50, 255, 128);
  SDL_Rect glow_rect = {
    left - neon_glow_offset,
    top - neon_glow_offset,
    arena_width + 2 * neon_glow_offset,
    arena_height + 2 * neon_glow_offset
};
  SDL_RenderDrawRect(renderer_, &glow_rect);

  SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
  SDL_Rect top_bar = {left, top - border_thickness, arena_width, border_thickness};
  SDL_Rect bottom_bar = {left, top + arena_height, arena_width, border_thickness};
  SDL_Rect left_bar = {left - border_thickness, top, border_thickness, arena_height};
  SDL_Rect right_bar = {left + arena_width, top, border_thickness, arena_height};

  SDL_RenderFillRect(renderer_, &top_bar);
  SDL_RenderFillRect(renderer_, &bottom_bar);
  SDL_RenderFillRect(renderer_, &left_bar);
  SDL_RenderFillRect(renderer_, &right_bar);
}


void Renderer::DrawGame(const client::GameState& game_state) const {
  DrawArenaBoundaries();

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
        static_cast<int>(x - camera_.x),
        static_cast<int>(y - camera_.y),
        50,
        50
    };
      SDL_SetRenderDrawColor(renderer_, 0, 0, 255, 255);
    } else if (i < projectiles.size() && projectiles[i].has_value()) {
      rect = SDL_Rect{
        static_cast<int>(x - camera_.x),
        static_cast<int>(y - camera_.y),
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
