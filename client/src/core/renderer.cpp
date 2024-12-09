#include <stdexcept>

#include "client/core/renderer.hpp"

// Shaders pour le rendu
auto vertex_shader_source = R"(
#version 330 core

layout (location = 0) in vec2 position;

uniform mat4 projection;

void main() {
    gl_Position = projection * vec4(position, 0.0, 1.0);
}

)";

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

auto neon_bar_horizontal_shader = R"(
#version 330 core

out vec4 FragColor;

uniform float time;

// Ajout des uniformes pour la position et la taille
uniform vec2 rect_position; // Position du rectangle
uniform vec2 rect_size;     // Taille du rectangle

const float zoom = 3.0; // Facteur de zoom pré-défini

void main(void) {
    // Position du pixel en cours
    vec2 pixel_pos = gl_FragCoord.xy;

    // Vérifier si le pixel est dans le rectangle
    if (pixel_pos.x < rect_position.x || pixel_pos.x > rect_position.x + rect_size.x ||
        pixel_pos.y < rect_position.y || pixel_pos.y > rect_position.y + rect_size.y) {
        // En dehors du rectangle : ne rien dessiner (transparent/noir)
        FragColor = vec4(0.0);
        return;
    }

    // Normaliser la position dans le rectangle
    vec2 uPos = (pixel_pos - rect_position) / rect_size;

    // Appliquer le zoom
    uPos /= zoom;

    uPos.x += (1.0 - 1.0 / zoom) * 0.5;
    uPos.y += (1.0 - 1.0 / zoom) * 0.5;

    // Centrer l'origine (0, 0) au centre du rectangle
    uPos.x -= 0.5;
    uPos.y -= 0.5;

    vec3 color = vec3(0.0); // Couleur initiale
    float horizColor = 0.0;

    // Appliquer le décalage sinusoïdal uniquement sur l'axe vertical
    float t = time * 0.5; // Ajustez la vitesse du temps
    float wave = sin(uPos.x * 10.0 + t) * 0.005; // Onde uniquement sur l'axe vertical
    uPos.y += wave;

    // Calculer une intensité en fonction de la position (dépend de uPos.y)
    float fTemp = pow(abs(1.0 / (uPos.y * 150.0)), 1.5);

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




auto neon_bar_vertical_shader = R"(
#version 330 core

out vec4 FragColor;

uniform float time;

// Ajout des uniformes pour la position et la taille
uniform vec2 rect_position; // Position du rectangle
uniform vec2 rect_size;     // Taille du rectangle

const float zoom = 3.0; // Facteur de zoom pré-défini

void main(void) {
    // Position du pixel en cours
    vec2 pixel_pos = gl_FragCoord.xy;

    // Vérifier si le pixel est dans le rectangle
    if (pixel_pos.x < rect_position.x || pixel_pos.x > rect_position.x + rect_size.x ||
        pixel_pos.y < rect_position.y || pixel_pos.y > rect_position.y + rect_size.y) {
        // En dehors du rectangle : ne rien dessiner (transparent/noir)
        FragColor = vec4(0.0);
        return;
    }

    // Normaliser la position dans le rectangle
    vec2 uPos = (pixel_pos - rect_position) / rect_size;

    // Appliquer le zoom
    uPos /= zoom;

    uPos.x += (1.0 - 1.0 / zoom) * 0.5;
    uPos.y += (1.0 - 1.0 / zoom) * 0.5;


    // Centrer l'origine (0, 0) au centre du rectangle
    uPos.x -= 0.5;
    uPos.y -= 0.5;

    vec3 color = vec3(0.0); // Couleur initiale
    float horizColor = 0.0;

    // Appliquer le décalage sinusoïdal uniquement sur l'axe horizontal
    float t = time * 0.5; // Ajustez la vitesse du temps
    float wave = sin(uPos.y * 10.0 + t) * 0.005; // Onde uniquement sur l'axe horizontal
    uPos.x += wave;

    // Calculer une intensité en fonction de la position
    //  float fTemp = abs(1.0 / (uPos.x * 100.0));

    float fTemp = pow(abs(1.0 / (uPos.x * 150.0)), 1.5);

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


auto starguy_shader_source = R"(
#version 330 core

out vec4 FragColor;

uniform float time;
uniform vec2 resolution;
uniform vec2 rect_position; // Position du rectangle


// hash without sine
float hash11(float p) {
    vec3 p3 = fract(vec3(p) * vec3(.1031, .11369, .13787));
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}

// 1D smooth noise
float snoise1d(float f) {
    return mix(
        hash11(floor(f)),
        hash11(floor(f + 1.0)),
        smoothstep(0.0, 1.0, fract(f))
    );
}

// Star shape (2D distance estimate)
float StarDE(vec2 p, float n, float r, float i) {
    float rep = floor(-atan(p.x, p.y) * (n / 6.28) + 0.5) / (n / 6.28);
    float s, c;
    s = sin(rep);
    c = cos(rep);
    p = mat2(c, -s, s, c) * p;
    float a = (i + 1.0) * 3.14 / n;
    s = sin(a);
    c = cos(a);
    p = mat2(c, -s, s, c) * vec2(-abs(p.x), p.y - r);
    return length(max(vec2(0.0), p));
}

// StarDE with eyes
float Starguy(vec2 p, float n, float r, float i, vec2 l) {
    float b = pow(abs(fract(0.087 * time + 0.1) - 0.5) * 2.0, 72.0);
    vec2 p2 = p + l;

    return max(
        StarDE(p, n, r, i),
        -length(vec2(
            min(0.0, -abs(abs(p2.x) - r * 0.2) + r * b * 0.1),
            min(0.0, -abs(p2.y) + r * (1.0 - b) * 0.1)
        )) + r * 0.11
    );
}

void main(void) {
    vec2 pixel_pos = gl_FragCoord.xy;

    if (pixel_pos.x < rect_position.x - 50.0 || pixel_pos.x > rect_position.x + resolution.x + 50.0 ||
      pixel_pos.y < rect_position.y - 50.0 || pixel_pos.y > rect_position.y + resolution.y + 50.0) {
      // En dehors de la zone élargie : ne rien dessiner (transparent/noir)
      FragColor = vec4(0.0);
      return;
    }

    // Position relative à rect_position
    vec2 p = (gl_FragCoord.xy - rect_position - resolution.xy / 2.0) / resolution.y;

    float t = 0.7 * time;
    vec2 p2 = p;
    p2.y += 0.025 * sin(4.0 * t);

    p2 = p2 / dot(p2, p2) - 0.17 * vec2(sin(t), cos(4.0 * t));
    p2 = p2 / dot(p2, p2);

    vec2 look = 0.02 * vec2(cos(0.71 * t), sin(0.24 * t));

    float star = Starguy(p2, 5.0, 0.27, 0.7, look);

    float rad = pow(Starguy(p, 5.0, 0.27, 0.7, look), 0.5);
    rad = snoise1d(24.0 * rad - 2.0 * time) + 0.5 * snoise1d(48.0 * rad - 4.0 * time);

    vec3 col = mix(
        vec3(1.0),
        vec3(-0.1, 0.0, 0.0),

     //   vec3(-0.1, 0.0, 0.2),
        clamp(star / 0.01, 0.0, 1.0)
    ) + 4.5 * vec3(1.0, 0.5, 0.23) * (1.05 - pow(star, 0.05)) * (1.0 - 0.04 * rad);


    // Distance relative au centre (normalisée)
    vec2 center = rect_position + resolution / 2.0;
    float distance_to_center = length((gl_FragCoord.xy - center) / resolution);
  //  float attenuation = 1 - clamp(distance_to_center, 0.0, 1.0); // Va de 1.0 (centre) à 0.0 (bord)
    float attenuation = pow(1.0 - clamp(distance_to_center, 0.0, 1.0), 2.5);


    // Appliquer l'atténuation
    col *= attenuation;

    FragColor = vec4(col, 1.0);
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
//  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);

  // Initialize OpenGL resources
  InitOpenGL();

  // Initialize camera
 // camera_.projection_matrix = glm::ortho(0.0f, static_cast<float>(width_), 0.0f, static_cast<float>(height_));

  camera_.projection_matrix = glm::ortho(0.0f, static_cast<float>(width_), static_cast<float>(height_), 0.0f);
  camera_.position = glm::vec2(0.0f, 0.0f);
}


void Renderer::InitOpenGL() {
  shader_program_ = LoadShaders(vertex_shader_source, fragment_shader_source);

  neon_bar_horizontal_program_ = LoadShaders(vertex_shader_source, neon_bar_horizontal_shader); // Utilisez la fonction `LoadShaders`
  neon_bar_vertical_program_ = LoadShaders(vertex_shader_source, neon_bar_vertical_shader); // Utilisez la fonction `LoadShaders`
  starguy_program_ = LoadShaders(vertex_shader_source, starguy_shader_source); // Utilisez la fonction `LoadShaders`

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
   // camera_.position = glm::vec2(0, 0);

void Renderer::UpdateCamera(const client::GameState& game_state) {
  // Met à jour la position de la caméra en fonction du joueur
  auto& registry = game_state.GetRegistry();
  auto& players = registry.get_components<Player>();
  auto& positions = registry.get_components<Position>();

  for (size_t i = 0; i < players.size(); ++i) {
    if (players[i].has_value() && positions[i].has_value()) {
      const auto& [x, y] = *positions[i];
      std::cout << "Player position: " << x << ", " << y << '\n';
      camera_.position = glm::vec2(x - width_ / 2.0f, -y - height_ / 2.0f);

     // camera_.projection_matrix = glm::ortho(0.0f, static_cast<float>(width_),
     //                                        static_cast<float>(height_), 0.0f);

    //  camera_.projection_matrix = glm::ortho(
    //      camera_.position.x, camera_.position.x + width_,
    //      camera_.position.y + height_, camera_.position.y);
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

void Renderer::DrawHorizontalNeonBar(const glm::vec2& map_position, const glm::vec2& size) const {
  glUseProgram(neon_bar_horizontal_program_);

  // Passer la matrice de projection au shader
  glUniformMatrix4fv(glGetUniformLocation(neon_bar_horizontal_program_, "projection"), 1, GL_FALSE, glm::value_ptr(camera_.projection_matrix));

  // Passer les uniformes pour le temps et la résolution
  glUniform1f(glGetUniformLocation(neon_bar_horizontal_program_, "time"), static_cast<float>(SDL_GetTicks()) / 500.0f);
  glUniform2f(glGetUniformLocation(neon_bar_horizontal_program_, "resolution"), static_cast<float>(width_), static_cast<float>(height_));

  glm::vec2 screen_position = map_position - camera_.position;

  // Passer la position et la taille corrigées du rectangle au shader
  glUniform2f(glGetUniformLocation(neon_bar_horizontal_program_, "rect_position"), screen_position.x, screen_position.y);

  // Passer la position et la taille du rectangle au shader
 // glUniform2f(glGetUniformLocation(neon_bar_horizontal_program_, "rect_position"), position.x, position.y);
  glUniform2f(glGetUniformLocation(neon_bar_horizontal_program_, "rect_size"), size.x, size.y);

  float vertices[] = {
    0.0f, static_cast<float>(height_),  // Coin supérieur gauche
    static_cast<float>(width_), static_cast<float>(height_),  // Coin supérieur droit
    static_cast<float>(width_), 0.0f,  // Coin inférieur droit
    0.0f, 0.0f,  // Coin inférieur gauche
  };


  // Envoyer les données des sommets au GPU
  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

  // Dessiner le rectangle
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glUseProgram(0);
}

void Renderer::DrawVerticalNeonBar(const glm::vec2& map_position, const glm::vec2& size) const {
  glUseProgram(neon_bar_vertical_program_);

  // Passer la matrice de projection au shader
  glUniformMatrix4fv(glGetUniformLocation(neon_bar_vertical_program_, "projection"), 1, GL_FALSE, glm::value_ptr(camera_.projection_matrix));

  // Passer les uniformes pour le temps et la résolution
  glUniform1f(glGetUniformLocation(neon_bar_vertical_program_, "time"), static_cast<float>(SDL_GetTicks()) / 500.0f);
  glUniform2f(glGetUniformLocation(neon_bar_vertical_program_, "resolution"), static_cast<float>(width_), static_cast<float>(height_));

  glm::vec2 screen_position = map_position - camera_.position;

  // Passer la position et la taille corrigées du rectangle au shader
  glUniform2f(glGetUniformLocation(neon_bar_vertical_program_, "rect_position"), screen_position.x, screen_position.y);

  // Passer la position et la taille du rectangle au shader
  // glUniform2f(glGetUniformLocation(neon_bar_vertical_program_, "rect_position"), position.x, position.y);
  glUniform2f(glGetUniformLocation(neon_bar_vertical_program_, "rect_size"), size.x, size.y);

  float vertices[] = {
    0.0f, static_cast<float>(height_),  // Coin supérieur gauche
    static_cast<float>(width_), static_cast<float>(height_),  // Coin supérieur droit
    static_cast<float>(width_), 0.0f,  // Coin inférieur droit
    0.0f, 0.0f,  // Coin inférieur gauche
  };


  // Envoyer les données des sommets au GPU
  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

  // Dessiner le rectangle
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glUseProgram(0);
}

/*
void Renderer::DrawVisibleBar(const glm::vec2& position, const glm::vec2& size) const {
  // Calcul des limites visibles en X
  float visible_start_x = position.x; // Garder la largeur complète
  float visible_width = size.x;       // Toujours utiliser la largeur initiale

  // Calcul des limites visibles en Y
  float visible_start_y = std::max(position.y, camera_.position.y);
  float visible_end_y = std::min(position.y + size.y, camera_.position.y + height_);
  float visible_height = visible_end_y - visible_start_y;

  if (visible_width > 0.0f && visible_height > 0.0f) {
    // Position de départ pour dessiner (corrigée pour Y uniquement)
    glm::vec2 rect_position = {visible_start_x, visible_start_y};
    // Taille corrigée : largeur inchangée, hauteur ajustée
    glm::vec2 rect_size = {visible_width, visible_height};

    std::cout << "Visible bar: " << rect_position.x << ", " << rect_position.y
              << ", " << rect_size.x << ", " << rect_size.y << '\n';

    // Dessiner avec la taille corrigée
    DrawNeonRectangle(rect_position, rect_size);
  }
}
*/

void Renderer::DrawVisibleHorizontalBar(const glm::vec2& position, const glm::vec2& size) const {
  // Calculer les limites visibles en Y
  float visible_start_y = std::max(position.y, camera_.position.y);
  float visible_end_y = std::min(position.y + size.y, camera_.position.y + height_);
  float visible_height = visible_end_y - visible_start_y;

  // Vérifier si le rectangle est visible
  if (visible_height > 0.0f) {
    // Si visible, dessiner la barre
    glm::vec2 rect_position = position;
    glm::vec2 rect_size = size;

    std::cout << "Visible horizontal bar: " << rect_position.x << ", " << rect_position.y
              << ", " << rect_size.x << ", " << rect_size.y << '\n';

    DrawHorizontalNeonBar(rect_position, rect_size);
  } else {
    // Si aucune partie n'est visible, ne pas dessiner
    std::cout << "Horizontal bar is not visible, skipping draw.\n";
  }
}

void Renderer::DrawVisibleVerticalBar(const glm::vec2& position, const glm::vec2& size) const {
  // Calculer les limites visibles en X
  float visible_start_x = std::max(position.x, camera_.position.x);
  float visible_end_x = std::min(position.x + size.x, camera_.position.x + width_);
  float visible_width = visible_end_x - visible_start_x;

  // Vérifier si le rectangle est visible
  if (visible_width > 0.0f) {
    // Si visible, dessiner la barre
    glm::vec2 rect_position = position;
    glm::vec2 rect_size = size;

    std::cout << "Visible vertical bar: " << rect_position.x << ", " << rect_position.y
              << ", " << rect_size.x << ", " << rect_size.y << '\n';

    DrawVerticalNeonBar(rect_position, rect_size);
  } else {
    // Si aucune partie n'est visible, ne pas dessiner
    std::cout << "Vertical bar is not visible, skipping draw.\n";
  }
}


void Renderer::DrawVisibleBar(const glm::vec2& position, const glm::vec2& size) const {
  // Calculer les limites visibles en Y
  float visible_start_y = std::max(position.y, camera_.position.y);
  float visible_end_y = std::min(position.y + size.y, camera_.position.y + height_);
  float visible_height = visible_end_y - visible_start_y;

  // Vérifier si le rectangle est visible
  if (visible_height > 0.0f) {
    // Si visible, dessiner la barre avec la taille complète
    glm::vec2 rect_position = position; // Conserver la position initiale
    glm::vec2 rect_size = size;         // Conserver la taille initiale

    std::cout << "Visible bar: " << rect_position.x << ", " << rect_position.y
              << ", " << rect_size.x << ", " << rect_size.y << '\n';

    DrawHorizontalNeonBar(rect_position, rect_size);
  } else {
    // Si aucune partie n'est visible, ne pas dessiner
    std::cout << "Bar is not visible, skipping draw.\n";
  }
}

void Renderer::DrawStarguy(const glm::vec2& map_position, const glm::vec2& size) const {
  glUseProgram(starguy_program_);

  // Passer la matrice de projection au shader
  glUniformMatrix4fv(glGetUniformLocation(starguy_program_, "projection"), 1, GL_FALSE, glm::value_ptr(camera_.projection_matrix));

  // Passer les uniformes pour le temps et la résolution
  glUniform1f(glGetUniformLocation(starguy_program_, "time"), static_cast<float>(SDL_GetTicks()) / 1000.0f);
  glUniform2f(glGetUniformLocation(starguy_program_, "resolution"), static_cast<float>(size.x), static_cast<float>(size.y));

  glm::vec2 screen_position = map_position - camera_.position;

  screen_position.x += width_ / 2.0f;
  screen_position.x -= size.x / 2.0f;
  screen_position.y += height_ / 2.0f;
  screen_position.y -= size.y / 2.0f;

  // Passer les uniformes pour la position et la taille du rectangle
  glUniform2f(glGetUniformLocation(starguy_program_, "rect_position"), screen_position.x, screen_position.y);
 // glUniform2f(glGetUniformLocation(starguy_program_, "rect_size"), size.x, size.y);

  float vertices[] = {
    0.0f, static_cast<float>(height_),  // Coin supérieur gauche
    static_cast<float>(width_), static_cast<float>(height_),  // Coin supérieur droit
    static_cast<float>(width_), 0.0f,  // Coin inférieur droit
    0.0f, 0.0f  // Coin inférieur gauche
};

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
    // Dessiner plusieurs rectangles
    /*glm::vec2 rect_size1 = {12000.0f, 100.0f};
    glm::vec2 rect_position1 = {};
    DrawNeonRectangle(rect_position1, rect_size1);

    glm::vec2 rect_size2 = {150.0f, 100.0f};
    glm::vec2 rect_position2 = {400.0f, 300.0f};
    DrawNeonRectangle(rect_position2, rect_size2);

    glm::vec2 rect_size3 = {200.0f, 50.0f};
    glm::vec2 rect_position3 = {600.0f, 800.0f};
    DrawNeonRectangle(rect_position3, rect_size3);
    */

    glm::vec2 left_bar_position = {-200.0f, -2100.0f}; // Position de départ
    glm::vec2 left_bar_size = {400.0f, 2200.0f};  // Taille totale (largeur x hauteur)

    // Dessiner uniquement la partie visible
    DrawVisibleVerticalBar(left_bar_position, left_bar_size);

    glm::vec2 right_bar_position = {1800.0f, -2100.0f}; // Position de départ
    glm::vec2 right_bar_size = {400.0f, 2200.0f};  // Taille totale (largeur x hauteur)

    // Dessiner uniquement la partie visible
    DrawVisibleVerticalBar(right_bar_position, right_bar_size);

    glm::vec2 bottom_bar_position = {-100.0f, -2200.0f}; // Position de départ
    glm::vec2 bottom_bar_size = {2200.0f, 400.0f};  // Taille totale (largeur x hauteur)

    // Dessiner uniquement la partie visible
    DrawVisibleHorizontalBar(bottom_bar_position, bottom_bar_size);

    glm::vec2 top_bar_position = {-100.0f, -200.0f}; // Position de départ
    glm::vec2 top_bar_size = {2200.0f, 400.0f};  // Taille totale (largeur x hauteur)

    // Dessiner uniquement la partie visible
    DrawVisibleHorizontalBar(top_bar_position, top_bar_size);

  glm::vec2 stargui_position = {50.0f, -100.0f}; // Position de départ
  glm::vec2 stargui_size = {120.0f, 120.0f};  // Taille totale (largeur x hauteur)

  DrawStarguy(camera_.position, stargui_size);
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
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
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
  glDeleteProgram(neon_bar_horizontal_program_);
  glDeleteProgram(neon_bar_vertical_program_);
  glDeleteProgram(starguy_program_);

  if (gl_context_) {
    SDL_GL_DeleteContext(gl_context_);
    gl_context_ = nullptr;
  }
  if (window_) {
    SDL_DestroyWindow(window_);
    window_ = nullptr;
  }
}
