#ifndef TEXT_BOX_HPP
#define TEXT_BOX_HPP

#include <SDL2/SDL_ttf.h>

#include <functional>
#include <memory>
#include <string>
#include <string_view>

#include "text.hpp"
#include "ui_element.hpp"

/**
 * @brief A generic and customizable TextBox class.
 *
 * Allows for custom rendering strategies, such as borders, textures, or
 * background colors. Supports a cursor for user input navigation.
 */
class TextBox final : public UIElement {
 public:
  /**
   * @brief Constructs a TextBox.
   *
   * @param x               The X position of the TextBox.
   * @param y               The Y position of the TextBox.
   * @param width           The width of the TextBox.
   * @param height          The height of the TextBox.
   * @param text_obj        A Text object to be rendered inside the TextBox.
   * @param max_characters  The maximum number of characters allowed (0 for no limit).
   */
  TextBox(const int x, const int y, const int width, const int height,
          std::unique_ptr<Text> text_obj, const size_t max_characters = 0)
      : UIElement(x, y, width, height),
        text_obj_(std::move(text_obj)),
        is_focused_(false),
        cursor_position_(0),
        max_characters_(max_characters),
        render_strategy_(DefaultRenderStrategy) {
    if (text_obj_) {
      text_obj_->SetPosition(x + kPadding, y + kPadding);
    }
  }

  ~TextBox() override = default;

  /**
   * @brief Sets a custom render strategy for the TextBox.
   *
   * @param strategy The render strategy to use.
   */
  void SetRenderStrategy(std::function<void(SDL_Renderer*, const TextBox&)> strategy) {
    render_strategy_ = std::move(strategy);
  }

  void Render(SDL_Renderer* renderer) override {
    if (!IsVisible()) return;

    // Custom render strategy
    if (render_strategy_) {
      render_strategy_(renderer, *this);
    }

    // Render text
    if (text_obj_) {
      text_obj_->Render(renderer);
    }

    // Render cursor if focused
    if (is_focused_) {
      RenderCursor(renderer);
    }
  }

  void HandleInput(const SDL_Event& event) override {
    if (!IsEnabled() || !IsVisible()) return;

    switch (event.type) {
      case SDL_MOUSEBUTTONDOWN:
        HandleMouseClick(event.button.x, event.button.y);
        break;

      case SDL_TEXTINPUT:
        InsertText(event.text.text);
        break;

      case SDL_KEYDOWN:
        HandleKeyPress(event.key.keysym.sym);
        break;

      default:
        break;
    }
  }

  bool HasFocus() const { return is_focused_; }

  /**
   * @brief Updates the content of the text inside the TextBox.
   *
   * @param new_content The new content to set.
   */
  void SetContent(const std::string& new_content) {
    if (!text_obj_) return;
    const std::string truncated_content = TruncateContent(new_content);
    text_obj_->SetContent(truncated_content);
    cursor_position_ = static_cast<int>(truncated_content.size());
  }

  const std::string& GetContent() const {
    return text_obj_ ? text_obj_->GetContent() : kEmptyContent;
  }

  void SetMaxCharacters(const size_t max_characters) { max_characters_ = max_characters; }

  size_t GetMaxCharacters() const { return max_characters_; }

 private:
  static void DefaultRenderStrategy(SDL_Renderer* renderer, const TextBox& text_box) {
    // Default: Draw a white border
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &text_box.GetBounds());
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  }

  void RenderCursor(SDL_Renderer* renderer) const {
    const std::string& content = text_obj_->GetContent();
    int text_width = 0, text_height = 0;

    if (!content.empty()) {
      // Measure text width up to the cursor
      TTF_SizeText(text_obj_->GetFont(), content.substr(0, cursor_position_).c_str(),
                   &text_width, &text_height);
    }

    const int cursor_x = GetX() + kPadding + text_width;
    const int cursor_y = GetY() + kPadding;

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    const SDL_Rect cursor_rect = {cursor_x, cursor_y, 2, text_height};
    SDL_RenderFillRect(renderer, &cursor_rect);
  }

  void HandleMouseClick(const int mouse_x, const int mouse_y) {
    is_focused_ = IsPointInside(mouse_x, mouse_y);
  }

  void InsertText(const std::string_view text) {
    if (!is_focused_ || !text_obj_) return;

    if (std::string updated_content = text_obj_->GetContent();
        max_characters_ == 0 || updated_content.size() + text.size() <= max_characters_) {
      updated_content.insert(cursor_position_, text);
      text_obj_->SetContent(updated_content);
      cursor_position_ += static_cast<int>(text.size());
    }
  }

  void HandleKeyPress(const SDL_Keycode key) {
    if (!is_focused_ || !text_obj_) return;

    std::string updated_content = text_obj_->GetContent();

    switch (key) {
      case SDLK_BACKSPACE:
        if (cursor_position_ > 0) {
          updated_content.erase(cursor_position_ - 1, 1);
          text_obj_->SetContent(updated_content);
          cursor_position_--;
        }
        break;

      case SDLK_LEFT:
        if (cursor_position_ > 0) cursor_position_--;
        break;

      case SDLK_RIGHT:
        if (cursor_position_ < static_cast<int>(updated_content.size())) cursor_position_++;
        break;

      case SDLK_RETURN:
        is_focused_ = false;
        break;

      default:
        break;
    }
  }

  std::string TruncateContent(const std::string& content) const {
    if (max_characters_ == 0 || content.size() <= max_characters_) {
      return content;
    }
    return content.substr(0, max_characters_);
  }

  std::unique_ptr<Text> text_obj_;  ///< Text object inside the TextBox.
  bool is_focused_;                ///< Whether the TextBox is focused.
  int cursor_position_;            ///< Cursor position in the text.
  size_t max_characters_;          ///< Maximum characters allowed (0 = no limit).
  std::function<void(SDL_Renderer*, const TextBox&)> render_strategy_; ///< Custom rendering strategy.

  static constexpr int kPadding = 4;
  static const std::string kEmptyContent;
};

#endif  // TEXT_BOX_HPP
