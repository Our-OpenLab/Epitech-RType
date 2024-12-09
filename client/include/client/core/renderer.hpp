#ifndef RENDERER_HPP_
#define RENDERER_HPP_

#include <SDL.h>

#include "client/engine/game_state.hpp"

class Renderer {
public:
  Renderer(int width, int height, const std::string& title);
 ~Renderer();

  void DrawGame(const client::GameState& game_state) const;
  void Clear() const;
  void Present() const;
  void Shutdown();

private:
  SDL_Window* window_{nullptr};
  SDL_Renderer* renderer_{nullptr};
};

#endif  // RENDERER_HPP_
