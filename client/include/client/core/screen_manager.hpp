#ifndef SCREEN_MANAGER_HPP_
#define SCREEN_MANAGER_HPP_

#include <SDL.h>

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

  std::pair<float, float> NormalizeMousePosition(const int mouse_x,
                                                 const int mouse_y) const {
    return {
      static_cast<float>(mouse_x) / screen_width_,
      static_cast<float>(mouse_y) / screen_height_
  };
  }

private:
  int screen_width_{};
  int screen_height_{};
};

#endif  // SCREEN_MANAGER_HPP_
