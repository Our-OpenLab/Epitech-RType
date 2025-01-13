#ifndef UI_ELEMENT_HPP
#define UI_ELEMENT_HPP

#include <SDL2/SDL.h>

class UIElement {
public:
  UIElement(const int x, const int y, const int width, const int height)
      : bounds{x, y, width, height}, isVisible(true), isEnabled(true) {}

  virtual ~UIElement() = default;

  virtual void Render(SDL_Renderer* renderer) = 0;

  virtual void HandleInput(const SDL_Event& event) = 0;

  virtual void Update(float deltaTime) {}

  void SetPosition(const int newX, const int newY) {
    bounds.x = newX;
    bounds.y = newY;
  }

  void SetSize(const int newWidth, const int newHeight) {
    bounds.w = newWidth;
    bounds.h = newHeight;
  }

  void SetVisible(const bool visible) { isVisible = visible; }

  virtual void SetEnabled(const bool enabled) {
    isEnabled = enabled;
  }

  bool IsVisible() const {
    return isVisible;
  }

  bool IsEnabled() const {
    return isEnabled;
  }

  const SDL_Rect& GetBounds() const {
    return bounds;
  }

protected:
  SDL_Rect bounds;

  bool isVisible;
  bool isEnabled;

  bool IsPointInside(const int pointX, const int pointY) const {
    return pointX >= bounds.x && pointX <= bounds.x + bounds.w &&
           pointY >= bounds.y && pointY <= bounds.y + bounds.h;
  }
};

#endif // UI_ELEMENT_HPP