#ifndef RENDERER_HPP_
#define RENDERER_HPP_

#include <SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

#include "client/engine/game_state.hpp"

struct Camera {
  float x, y;
  int width, height;
};

class Renderer {
public:
  Renderer(int width, int height, const std::string& title);
  ~Renderer();
  void UpdateCamera(const client::GameState& game_state);

  std::pair<float, float> GetCameraPosition() const {
    return {camera_.x, camera_.y};
  }

  void DrawArenaBoundaries() const;
  void DrawGame(const client::GameState& game_state) const;
  void Clear() const;
  void Present() const;
  void Shutdown();

private:
  SDL_Window* window_{nullptr};
  SDL_Renderer* renderer_{nullptr};
  Camera camera_;
};

#endif  // RENDERER_HPP_
