#include <stdexcept>

#include "client/core/renderer.hpp"

// Shaders pour le rendu
auto vertex_shader_source = R"(
#version 330 core
layout (location = 0) in vec2 position;
uniform mat4 projection;
void main() {
    gl_Position = projection * vec4(position, 0.0, 1.0);
})";

// Fragment shader source
auto fragment_shader_source = R"(
#version 330 core
out vec4 FragColor;
uniform vec3 color;
void main() {
    FragColor = vec4(color, 1.0);
})";

/*
auto neon_fragment_shader_source = R"(
#version 330 core

out vec4 FragColor;

uniform float time;
uniform vec2 resolution;

void main(void) {
    // Calculer les coordonnées normalisées de l'écran
    vec2 p = (gl_FragCoord.xy * 2.0 - resolution) / min(resolution.x, resolution.y);

    // Couleur de base
    vec3 color = vec3(0.0, 0.3, 0.5);

    // Calcul de l'effet
    float f = 0.0;
    float PI = 3.141592;

    for (float i = 0.0; i < 20.0; i++) {
        float s = sin(time + i * PI / 20.0) * 0.8;
        float c = cos(time + i * PI / 20.0) * 0.8;
        f += 0.001 / (abs(p.x + c) * abs(p.y + s));
    }

    // Appliquer la couleur calculée
    FragColor = vec4(vec3(f * color), 1.0);
}

)";

*/


auto neon_fragment_shader_source = R"(
#version 330 core

out vec4 FragColor;

uniform float time;
uniform vec2 resolution;

void main(void) {
    // Normaliser les coordonnées de l'écran
    vec2 uPos = gl_FragCoord.xy / resolution.xy;

    // Centrer l'origine (0, 0) au centre de l'écran
    uPos.x -= 0.5; // Décalage horizontal pour centrer
    uPos.y -= 0.5; // Décalage vertical pour centrer

    vec3 color = vec3(0.0); // Couleur initiale
    float horizColor = 0.0;

    // Appliquer le décalage sinusoïdal uniquement sur l'axe horizontal
    float t = time * 0.5; // Ajustez la vitesse du temps
    float wave = sin(uPos.y * 10.0 + t) * 0.005; // Onde uniquement sur l'axe horizontal
    uPos.x += wave;

    // Calculer une intensité en fonction de la position
    float fTemp = abs(1.0 / (uPos.x * 100.0));
    horizColor += fTemp;

    // Ajouter des composantes de couleur
    color += vec3(
        fTemp * 0.8,  // Rouge
        fTemp * 0.2,  // Vert
        pow(fTemp, 0.99) * 1.5 // Bleu
    );

    // Appliquer la couleur finale
    FragColor = vec4(color, 1.0);
}

)";

Renderer::Renderer(int width, int height, const std::string& title)
    : width_(width), height_(height) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    throw std::runtime_error("Failed to initialize SDL: " + std::string(SDL_GetError()));
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  window_ = SDL_CreateWindow(
      title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

  if (!window_) {
    throw std::runtime_error("Failed to create SDL window: " + std::string(SDL_GetError()));
  }

  gl_context_ = SDL_GL_CreateContext(window_);
  if (!gl_context_) {
    throw std::runtime_error("Failed to create OpenGL context: " + std::string(SDL_GetError()));
  }

  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK) {
    throw std::runtime_error("Failed to initialize GLEW");
  }

  glViewport(0, 0, width_, height_);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Initialize OpenGL resources
  InitOpenGL();

  // Initialize camera
  camera_.projection_matrix = glm::ortho(0.0f, static_cast<float>(width_), static_cast<float>(height_), 0.0f);
  camera_.position = glm::vec2(0.0f, 0.0f);
}


void Renderer::InitOpenGL() {
  shader_program_ = LoadShaders(vertex_shader_source, fragment_shader_source);

  neon_shader_program_ = LoadShaders(vertex_shader_source, neon_fragment_shader_source); // Utilisez la fonction `LoadShaders`

  glGenVertexArrays(1, &vao_);
  glGenBuffers(1, &vbo_);

  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, nullptr, GL_DYNAMIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}



Renderer::~Renderer() {
  Shutdown();
}

GLuint Renderer::LoadShaders(const char* vertex_source, const char* fragment_source) {
  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_source, nullptr);
  glCompileShader(vertex_shader);
  CheckShaderCompileError(vertex_shader);

  GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_source, nullptr);
  glCompileShader(fragment_shader);
  CheckShaderCompileError(fragment_shader);

  GLuint program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);
  CheckProgramLinkError(program);

  // Log success
  std::cout << "Shaders compiled and linked successfully!" << std::endl;

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  return program;
}


void Renderer::CheckShaderCompileError(GLuint shader) {
  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char info_log[512];
    glGetShaderInfoLog(shader, 512, nullptr, info_log);
    throw std::runtime_error("Shader compile error: " + std::string(info_log));
  }
}

void Renderer::CheckProgramLinkError(GLuint program) {
  GLint success;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    char info_log[512];
    glGetProgramInfoLog(program, 512, nullptr, info_log);
    throw std::runtime_error("Program link error: " + std::string(info_log));
  }
}


void Renderer::UpdateCamera(const client::GameState& game_state) {
  // Met à jour la position de la caméra en fonction du joueur
  auto& registry = game_state.GetRegistry();
  auto& players = registry.get_components<Player>();
  auto& positions = registry.get_components<Position>();

  for (size_t i = 0; i < players.size(); ++i) {
    if (players[i].has_value() && positions[i].has_value()) {
      const auto& [x, y] = *positions[i];
      camera_.position = glm::vec2(x - width_ / 2.0f, y - height_ / 2.0f);
      camera_.projection_matrix = glm::ortho(
          camera_.position.x, camera_.position.x + width_,
          camera_.position.y + height_, camera_.position.y);
      return;
    }
  }
}

void Renderer::DrawArenaBoundaries() const {
  glUseProgram(shader_program_);

  float vertices[8] = {
      -camera_.position.x, -camera_.position.y,
      2000 - camera_.position.x, -camera_.position.y,
      2000 - camera_.position.x, 2000 - camera_.position.y,
      -camera_.position.x, 2000 - camera_.position.y,
  };

  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

  glUniformMatrix4fv(glGetUniformLocation(shader_program_, "projection"), 1, GL_FALSE, glm::value_ptr(camera_.projection_matrix));
  glUniform3f(glGetUniformLocation(shader_program_, "color"), 1.0f, 1.0f, 1.0f);

  glDrawArrays(GL_LINE_LOOP, 0, 4);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glUseProgram(0);
}

void Renderer::DrawRectangle(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color) const {
  glUseProgram(shader_program_);

  float vertices[] = {
    position.x, position.y,
    position.x + size.x, position.y,
    position.x + size.x, position.y + size.y,
    position.x, position.y + size.y,
};

  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

  glUniformMatrix4fv(glGetUniformLocation(shader_program_, "projection"), 1, GL_FALSE, glm::value_ptr(camera_.projection_matrix));
  glUniform3f(glGetUniformLocation(shader_program_, "color"), color.r, color.g, color.b);

  glDrawArrays(GL_LINE_LOOP, 0, 4);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glUseProgram(0);
}

void Renderer::DrawNeonRectangle(const glm::vec2& position, const glm::vec2& size) const {
  glUseProgram(neon_shader_program_);

  // Passer le temps au shader
  glUniform1f(glGetUniformLocation(neon_shader_program_, "time"), static_cast<float>(SDL_GetTicks()) / 1000.0f);

  // Passer la résolution de l'écran
  glUniform2f(glGetUniformLocation(neon_shader_program_, "resolution"), static_cast<float>(width_), static_cast<float>(height_));

  // Passer la matrice de projection
  glUniformMatrix4fv(glGetUniformLocation(neon_shader_program_, "projection"), 1, GL_FALSE, glm::value_ptr(camera_.projection_matrix));

  // Définir les sommets du rectangle
  float vertices[] = {
    position.x, position.y,
    position.x + size.x, position.y,
    position.x + size.x, position.y + size.y,
    position.x, position.y + size.y,
};

  // Envoyer les données au GPU
  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

  // Dessiner le rectangle
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glUseProgram(0);
}


void Renderer::DrawGame(const client::GameState& game_state) const {
 // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Dessiner un carré néon au centre
  glm::vec2 rect_position = { 100.0f, 100.0f }; // Position
  glm::vec2 rect_size = { 1000.0f, 800.0f };     // Taille

  DrawNeonRectangle(rect_position, rect_size);


}



/*

void Renderer::DrawGame(const client::GameState& game_state) const {
  // Utiliser OpenGL pour le rendu
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Obtenir la position de la caméra
  auto [camera_x, camera_y] = game_state.GetLocalPlayerPosition();//GetCameraPosition();

  // Obtenir les composants du GameState
  auto& registry = game_state.GetRegistry();
  auto& positions = registry.get_components<Position>();
  auto& players = registry.get_components<Player>();
  auto& projectiles = registry.get_components<Projectile>();

  // Dessiner les joueurs
  for (size_t i = 0; i < positions.size(); ++i) {
    if (!positions[i].has_value()) {
      continue;
    }

    const auto& position = *positions[i];

    glm::vec2 screen_position = {
      position.x - camera_x,
      position.y - camera_y
  };

    if (i < players.size() && players[i].has_value()) {
      DrawRectangle(screen_position, glm::vec2(50.0f, 50.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
    } else if (i < projectiles.size() && projectiles[i].has_value()) {
      DrawRectangle(screen_position, glm::vec2(20.0f, 10.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    }
  }

  // Swap des buffers pour mettre à jour l'écran
  SDL_GL_SwapWindow(window_);
}

*/

void Renderer::Clear() const {
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

 // glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::Present() const {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  SDL_GL_SwapWindow(window_);
}

void Renderer::Shutdown() {
  glDeleteVertexArrays(1, &vao_);
  glDeleteBuffers(1, &vbo_);
  glDeleteProgram(shader_program_);

  if (gl_context_) {
    SDL_GL_DeleteContext(gl_context_);
    gl_context_ = nullptr;
  }
  if (window_) {
    SDL_DestroyWindow(window_);
    window_ = nullptr;
  }
}
