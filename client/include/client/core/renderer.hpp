#ifndef RENDERER_HPP_
#define RENDERER_HPP_

#include <SDL2/SDL.h>
#include <string>

#include "game_state.hpp"

class Renderer {
public:
  Renderer(int width, int height, const std::string& title);
  ~Renderer();

  void draw_game(const GameState& game_state) const;

  void clear() const;

  void present() const;

  void shutdown();

private:
  SDL_Window* window_;
  SDL_Renderer* renderer_;
};

#endif  // RENDERER_HPP_
