#ifndef TEXT_LINK_HPP
#define TEXT_LINK_HPP

#include <functional>
#include <memory>
#include <string>

#include "text.hpp"

/**
 * @brief A class for interactive, link-like text elements.
 *
 * `TextLink` displays text with underline styling, supports hover effects,
 * and triggers a callback when clicked, behaving like a hyperlink.
 */
class TextLink final : public UIElement {
 public:
  using ClickCallback = std::function<void()>;

  /**
   * @brief Constructs a `TextLink` element.
   *
   * @param x         The X position of the text link.
   * @param y         The Y position of the text link.
   * @param content   The text content of the link.
   * @param font      The TTF_Font used for rendering the text.
   * @param normal_color The SDL_Color for the text in its normal state.
   * @param hover_color  The SDL_Color for the text when hovered.
   * @param renderer  The SDL_Renderer used for rendering the text.
   */
  TextLink(int x, int y, const std::string& content, TTF_Font* font,
           const SDL_Color& normal_color, const SDL_Color& hover_color,
           SDL_Renderer* renderer)
      : UIElement(x, y, 0, 0),
        normal_color_(normal_color),
        hover_color_(hover_color),
        is_hovered_(false),
        on_click_(nullptr) {
    text_ = std::make_unique<Text>(x, y, content, font, normal_color, renderer);
    SetSize(text_->GetBounds().w, text_->GetBounds().h);
  }

  /**
   * @brief Renders the link on the screen.
   *
   * @param renderer The SDL_Renderer used to draw the link.
   */
  void Render(SDL_Renderer* renderer) override {
    if (text_) {
      text_->Render(renderer);
      RenderUnderline(renderer);
    }
  }

  /**
   * @brief Handles mouse input for hover and click events.
   *
   * @param event The SDL_Event to process.
   */
  void HandleInput(const SDL_Event& event) override {
    if (event.type == SDL_MOUSEMOTION) {
      int mouse_x = event.motion.x;
      int mouse_y = event.motion.y;
      SDL_Rect bounds = GetBounds();
      bool is_inside = mouse_x >= bounds.x && mouse_x <= (bounds.x + bounds.w) &&
                       mouse_y >= bounds.y && mouse_y <= (bounds.y + bounds.h);

      if (is_inside && !is_hovered_) {
        is_hovered_ = true;
        text_->SetColor(hover_color_);
      } else if (!is_inside && is_hovered_) {
        is_hovered_ = false;
        text_->SetColor(normal_color_);
      }
    }

    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
      int mouse_x = event.button.x;
      int mouse_y = event.button.y;
      SDL_Rect bounds = GetBounds();

      if (mouse_x >= bounds.x && mouse_x <= (bounds.x + bounds.w) &&
          mouse_y >= bounds.y && mouse_y <= (bounds.y + bounds.h)) {
        if (on_click_) {
          on_click_();
        }
      }
    }
  }

  /**
   * @brief Sets the text content of the link.
   *
   * Updates the size and bounds of the link.
   *
   * @param content The new text content.
   */
  void SetContent(const std::string& content) {
    text_->SetContent(content);
    SetSize(text_->GetBounds().w, text_->GetBounds().h);
  }

  /**
   * @brief Sets the click callback.
   *
   * @param callback The function to call when the link is clicked.
   */
  void SetOnClick(ClickCallback callback) {
    on_click_ = std::move(callback);
  }

 private:
  std::unique_ptr<Text> text_;       ///< The underlying Text object.
  SDL_Color normal_color_;           ///< The text color in normal state.
  SDL_Color hover_color_;            ///< The text color when hovered.
  bool is_hovered_;                  ///< Whether the link is currently hovered.
  ClickCallback on_click_;           ///< The callback function for clicks.

  /**
   * @brief Renders an underline below the text to mimic a hyperlink style.
   *
   * @param renderer The SDL_Renderer used to draw the underline.
   */
  void RenderUnderline(SDL_Renderer* renderer) const {
    SDL_Rect bounds = GetBounds();
    SDL_SetRenderDrawColor(renderer, normal_color_.r, normal_color_.g,
                           normal_color_.b, normal_color_.a);
    if (is_hovered_) {
      SDL_SetRenderDrawColor(renderer, hover_color_.r, hover_color_.g,
                             hover_color_.b, hover_color_.a);
    }
    SDL_RenderDrawLine(renderer, bounds.x, bounds.y + bounds.h + 1,
                       bounds.x + bounds.w, bounds.y + bounds.h + 1);
  }
};

#endif  // TEXT_LINK_HPP
