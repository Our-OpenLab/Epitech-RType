#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <stdexcept>
#include <iostream>

struct Camera {
  glm::vec2 position{0.0f, 0.0f};
  glm::mat4 projection_matrix{1.0f};
};

class Renderer {
public:
  enum class RendererType { SDL, OpenGL };

  Renderer(int width, int height, const std::string& title, RendererType type = RendererType::SDL);
  ~Renderer();

  void SwitchToSDL(const std::string& title);
  void SwitchToOpenGL(const std::string& title);

  // General rendering
  void Clear() const;
  void Present() const;

  // SDL-specific functions
  void SetDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) const;
  SDL_Renderer* GetSDLRenderer() const { return sdlRenderer_; }

  // OpenGL-specific functions
  void UpdateCamera(const std::pair<float, float>& position);

  void DrawVisibleVerticalBar(const glm::vec2& position, const glm::vec2& size) const;
  void DrawVisibleHorizontalBar(const glm::vec2& position, const glm::vec2& size) const;
  void DrawStarguy(const glm::vec2& position, const glm::vec2& size) const;
  void DrawProjectile(const glm::vec2& position, const glm::vec2& size) const;
  void DrawEnemy(const glm::vec2& position, const glm::vec2& size) const;

private:
  RendererType currentRenderer_;

  GLuint neon_bar_horizontal_program_{0};
  GLuint neon_bar_vertical_program_{0};
  GLuint starguy_program_{0};
  GLuint projectile_program_{0};
  GLuint enemy_program_{0};

  // SDL components
  SDL_Window* window_{nullptr};
  SDL_Renderer* sdlRenderer_{nullptr};
  SDL_GLContext gl_context_{nullptr};

  // OpenGL components
  GLuint vao_{0};
  GLuint vbo_{0};
  GLuint shader_program_{0};

  int width_;
  int height_;

  Camera camera_;

  void InitSDL(const std::string& title);
  void InitOpenGL(const std::string& title);
  void CleanupSDL();
  void CleanupOpenGL();

  GLuint LoadShaders(const char* vertex_source, const char* fragment_source);
  void CheckShaderCompileError(GLuint shader);
  void CheckProgramLinkError(GLuint program);
};

#endif // RENDERER_HPP
