#ifndef SCREEN_MANAGER_HPP_
#define SCREEN_MANAGER_HPP_

#include <SDL.h>
#include <iostream>
#include <utility>

class ScreenManager {
public:
  void InitializeScreenDimensions() {
    SDL_DisplayMode display_mode;
    if (SDL_GetCurrentDisplayMode(0, &display_mode) == 0) {
      screen_width_ = display_mode.w;
      screen_height_ = display_mode.h;
    } else {
      std::cerr << "Failed to get display mode: " << SDL_GetError() << '\n';
    }
  }

  void UpdateScreenDimensions(const SDL_Event& event) {
    if (event.type == SDL_WINDOWEVENT &&
        (event.window.event == SDL_WINDOWEVENT_RESIZED ||
         event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)) {
      SDL_DisplayMode display_mode;
      if (SDL_GetCurrentDisplayMode(0, &display_mode) == 0) {
        screen_width_ = display_mode.w;
        screen_height_ = display_mode.h;
      }
         }
  }

  std::pair<int, int> GetScreenDimensions() const {
    return {screen_width_, screen_height_};
  }

  std::pair<float, float> MouseToWorldCoordinates(const float mouse_x,
                                                  const float mouse_y) {
    constexpr float screen_width = 1280.0f;
    constexpr float screen_height = 960.0f;

    constexpr float half_screen_width = screen_width / 2.0f;
    constexpr float half_screen_height = screen_height / 2.0f;

    float world_mouse_x = -half_screen_width + (mouse_x / screen_width) * screen_width;
    float world_mouse_y = -half_screen_height + (mouse_y / screen_height) * screen_height;

    return {world_mouse_x, world_mouse_y};
  }

private:
  int screen_width_{};
  int screen_height_{};
};

#endif  // SCREEN_MANAGER_HPP_
