#ifndef BUTTON_HPP
#define BUTTON_HPP

#include <functional>
#include <unordered_map>
#include <memory>
#include <SDL2/SDL.h>
#include "ui_element.hpp"
#include "text.hpp"

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
        current_state_(ButtonState::Normal),
        on_click_(nullptr) {}

  void SetRenderStrategy(const ButtonState state, std::function<void(SDL_Renderer*, const Button&)> strategy) {
    render_strategies_[state] = std::move(strategy);
  }

  void Render(SDL_Renderer* renderer) override {
    if (!IsVisible()) return;

    // Use the render strategy for the current state if available
    if (render_strategies_.count(current_state_) > 0) {
      render_strategies_[current_state_](renderer, *this);
    } else {
      RenderBackground(renderer);
    }

    RenderContent(renderer);
  }

  void HandleInput(const SDL_Event& event) override {
    if (!IsEnabled() || !IsVisible()) return;

    const int mouse_x = event.motion.x;
    const int mouse_y = event.motion.y;
    const bool is_mouse_inside = IsPointInside(mouse_x, mouse_y);

    switch (event.type) {
      case SDL_MOUSEMOTION:
        current_state_ = is_mouse_inside ? ButtonState::Hover : ButtonState::Normal;
        break;

      case SDL_MOUSEBUTTONDOWN:
        if (is_mouse_inside && event.button.button == SDL_BUTTON_LEFT) {
          current_state_ = ButtonState::Pressed;
        }
        break;

      case SDL_MOUSEBUTTONUP:
        if (current_state_ == ButtonState::Pressed && is_mouse_inside) {
          current_state_ = ButtonState::Hover;
          if (on_click_) on_click_();
        } else {
          current_state_ = ButtonState::Normal;
        }
        break;

      default:
        break;
    }
  }

  void SetOnClick(std::function<void()> callback) {
    on_click_ = std::move(callback);
  }

  void SetEnabled(const bool enabled) override {
    UIElement::SetEnabled(enabled);
    current_state_ = enabled ? ButtonState::Normal : ButtonState::Disabled;
  }

 protected:
  virtual void RenderBackground(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);  // Default gray
    SDL_RenderFillRect(renderer, &GetBounds());
  }

  virtual void RenderContent(SDL_Renderer* /*renderer*/) {
    // No additional content in base class
  }

 private:
  ButtonState current_state_;
  std::unordered_map<ButtonState, std::function<void(SDL_Renderer*, const Button&)>> render_strategies_;
  std::function<void()> on_click_;
};

class TextButton final : public Button {
 public:
  TextButton(const int x, const int y, const int width, const int height,
             std::unique_ptr<Text> text_obj)
      : Button(x, y, width, height),
        text_(std::move(text_obj)) {
    UpdateTextPosition();
  }

  void SetTextObject(std::unique_ptr<Text> text_obj) {
    text_ = std::move(text_obj);
    UpdateTextPosition();
  }

 protected:
  void RenderContent(SDL_Renderer* renderer) override {
    if (text_) {
      text_->Render(renderer);
    }
  }

 private:
  std::unique_ptr<Text> text_;

  void UpdateTextPosition() const {
    if (text_) {
      const int text_width = text_->GetWidth();
      const int text_height = text_->GetHeight();
      const int center_x = GetX() + (GetWidth() - text_width) / 2;
      const int center_y = GetY() + (GetHeight() - text_height) / 2;
      text_->SetPosition(center_x, center_y);
    }
  }
};

#endif  // BUTTON_HPP
