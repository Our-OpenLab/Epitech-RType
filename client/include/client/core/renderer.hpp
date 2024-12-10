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

  void UpdateCamera(const client::GameState& game_state);
  void DrawArenaBoundaries() const;
  void DrawGame(const client::GameState& game_state) const;

  void DrawRectangle(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color) const;
  void DrawNeonRectangleShader(const glm::vec2& position, const glm::vec2& size, const glm::vec3& neon_color, float glow_intensity, float glow_radius) const;
  void DrawNeonRectangle(const glm::vec2& position, const glm::vec2& size) const;


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

  GLuint neon_shader_program_{0};

  void InitOpenGL();
  GLuint LoadShaders(const char* vertex_source, const char* fragment_source);
  void CheckShaderCompileError(GLuint shader);
  void CheckProgramLinkError(GLuint program);
};

#endif  // RENDERER_HPP_
