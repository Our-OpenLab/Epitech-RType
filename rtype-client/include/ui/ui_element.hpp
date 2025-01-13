#ifndef UI_ELEMENT_HPP
#define UI_ELEMENT_HPP

#include <SDL2/SDL.h>

class UIElement {
public:
  UIElement(const int x, const int y, const int width, const int height)
    : bounds_{x, y, width, height}, visible_(true), enabled_(true) {}

  virtual ~UIElement() = default;

  // Accessors for position and size
  int GetX() const { return bounds_.x; }
  int GetY() const { return bounds_.y; }
  int GetWidth() const { return bounds_.w; }
  int GetHeight() const { return bounds_.h; }

  // Setters for position and size
  void SetPosition(const int x, const int y) {
    bounds_.x = x;
    bounds_.y = y;
  }

  void SetSize(const int width, const int height) {
    bounds_.w = width;
    bounds_.h = height;
  }

  const SDL_Rect& GetBounds() const { return bounds_; }

  // Visibility
  bool IsVisible() const { return visible_; }
  void SetVisible(const bool value) { visible_ = value; }

  // Enable/Disable interaction
  bool IsEnabled() const { return enabled_; }
  virtual void SetEnabled(const bool value) { enabled_ = value; }

  bool IsPointInside(const int x, const int y) const {
    return x >= bounds_.x && x <= bounds_.x + bounds_.w &&
           y >= bounds_.y && y <= bounds_.y + bounds_.h;
  }

  // Pure virtual methods for rendering and input handling
  virtual void Render(SDL_Renderer* renderer) = 0;
  virtual void HandleInput(const SDL_Event& event) = 0;

private:
  SDL_Rect bounds_; ///< The position and size of the element.
  bool visible_;    ///< Whether the element is visible.
  bool enabled_;    ///< Whether the element can receive input.
};

#endif // UI_ELEMENT_HPP
