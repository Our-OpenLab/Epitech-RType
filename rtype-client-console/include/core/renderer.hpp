#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <string>
#include <SDL2/SDL.h>

/**
 * @brief Simple SDL-based renderer manager.
 *
 * This class handles:
 *  - SDL initialization and shutdown.
 *  - Creation of an SDL window.
 *  - Creation of an SDL_Renderer for 2D drawing.
 *  - Basic methods for clearing and presenting the screen.
 */
class Renderer {
public:
  /**
   * @brief Constructor that initializes SDL, creates a window and a renderer.
   *
   * @param title  The window title.
   * @param width  The window width in pixels.
   * @param height The window height in pixels.
   */
  Renderer(const std::string& title, int width, int height);

  /**
   * @brief Destructor that cleans up the SDL objects and quits SDL.
   */
  ~Renderer();

  /**
     * @brief Clears the render target with a default color.
     */
  void Clear();

  /**
     * @brief Presents the rendered scene on the window.
     */
  void Present();

  /**
     * @brief Sets the draw color for subsequent rendering operations.
     *
     * @param r Red component   [0..255]
     * @param g Green component [0..255]
     * @param b Blue component  [0..255]
     * @param a Alpha component [0..255]
     */
  void SetDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);

  /**
   * @brief Returns the raw SDL_Renderer pointer for more advanced operations.
   */
  SDL_Renderer* GetSDLRenderer() const { return sdlRenderer; }

  /**
     * @brief Returns the raw SDL_Window pointer for more advanced operations.
     */
  SDL_Window* GetWindow() const { return window; }

private:
  SDL_Window*   window;      ///< The main application window.
  SDL_Renderer* sdlRenderer; ///< The hardware-accelerated 2D renderer.

  int windowWidth;           ///< Width of the window in pixels.
  int windowHeight;          ///< Height of the window in pixels.
};

#endif // RENDERER_HPP
