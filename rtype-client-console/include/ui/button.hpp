#ifndef BUTTON_HPP
#define BUTTON_HPP

#include <functional>

#include "ui_element.hpp"

enum class ButtonState {
  Normal,
  Hover,
  Pressed,
  Disabled
};

class Button : public UIElement {
public:
  Button(const int x, const int y, const int width, const int height)
        : UIElement(x, y, width, height),
          currentState(ButtonState::Normal),
          onClick(nullptr) {}

  void SetTexture(const ButtonState state, SDL_Texture* texture) {
    textures[state] = texture;
  }

  void Render(SDL_Renderer* renderer) override {
    if (IsVisible()) {
      if (SDL_Texture* texture = textures[currentState]) {
        SDL_RenderCopy(renderer, texture, nullptr, &bounds);
      }
    }
  }

  void HandleInput(const SDL_Event& event) override {
    if (!IsEnabled() || !IsVisible()) {
      return;
    }

    const int mouseX = event.motion.x;
    const int mouseY = event.motion.y;
    const bool isMouseInside = IsPointInside(mouseX, mouseY);

    switch (event.type) {
      case SDL_MOUSEMOTION:
        currentState = isMouseInside ? ButtonState::Hover : ButtonState::Normal;
      break;

      case SDL_MOUSEBUTTONDOWN:
        if (isMouseInside && event.button.button == SDL_BUTTON_LEFT) {
          currentState = ButtonState::Pressed;
        }
      break;

      case SDL_MOUSEBUTTONUP:
        if (currentState == ButtonState::Pressed && isMouseInside) {
          currentState = ButtonState::Hover;
          if (onClick) {
            onClick();
          }
        } else {
          currentState = ButtonState::Normal;
        }
      break;

      default:
        break;
    }
  }

  void Update(float deltaTime) override {
    // Optional: Implement animations or state transitions if needed.
  }

  void SetOnClick(std::function<void()> callback) {
    onClick = std::move(callback);
  }

  void SetEnabled(const bool enabled) override {
    UIElement::SetEnabled(enabled);
    currentState = enabled ? ButtonState::Normal : ButtonState::Disabled;
  }

private:
  ButtonState currentState;
  std::unordered_map<ButtonState, SDL_Texture*> textures;
  std::function<void()> onClick;
};

#endif // BUTTON_HPP
