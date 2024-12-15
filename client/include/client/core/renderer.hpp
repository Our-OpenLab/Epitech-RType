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
  glm::vec2 position{0.0f, 0.0f};
  glm::mat4 projection_matrix{1.0f};
};

class Renderer {
public:
  Renderer(int width, int height, const std::string& title);
  ~Renderer();

  void UpdateCamera(const std::pair<float, float>& position);
  void DrawArenaBoundaries() const;
  void DrawGame(const client::GameState& game_state) const;

  void DrawVisibleBar(const glm::vec2& position, const glm::vec2& size) const;
  void DrawHorizontalNeonBar(const glm::vec2& map_position, const glm::vec2& size) const;
  void DrawVerticalNeonBar(const glm::vec2& map_position, const glm::vec2& size) const;
  void DrawVisibleHorizontalBar(const glm::vec2& position, const glm::vec2& size) const;
  void DrawVisibleVerticalBar(const glm::vec2& position, const glm::vec2& size) const;
  void DrawStarguy(const glm::vec2& map_position, const glm::vec2& size) const;
  void DrawProjectile(const glm::vec2& map_position, const glm::vec2& size) const;
  void DrawEnemy(const glm::vec2& map_position, const glm::vec2& size) const;


  void Clear() const;
  void Present() const;
  void Shutdown();

private:
  SDL_Window* window_{nullptr};
  SDL_GLContext gl_context_{nullptr};

  Camera camera_;
  int width_;
  int height_;

  GLuint vao_{0};
  GLuint vbo_{0};
  GLuint shader_program_{0};

  GLuint neon_bar_horizontal_program_{0};
  GLuint neon_bar_vertical_program_{0};
  GLuint starguy_program_{0};
  GLuint projectile_program_{0};
  GLuint enemy_program_{0};

  void InitOpenGL();
  GLuint LoadShaders(const char* vertex_source, const char* fragment_source);
  void CheckShaderCompileError(GLuint shader);
  void CheckProgramLinkError(GLuint program);
};

#endif  // RENDERER_HPP_
