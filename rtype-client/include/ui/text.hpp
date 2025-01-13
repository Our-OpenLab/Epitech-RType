#ifndef TEXT_HPP
#define TEXT_HPP

#include <SDL2/SDL_ttf.h>

#include <iostream>
#include <string>

#include "ui_element.hpp"

/**
 * @brief A class for rendering text as a UIElement.
 *
 * The Text class uses SDL_ttf to render text content with a specified font
 * and color. It updates the texture dynamically whenever the content, font,
 * or color changes.
 */
class Text final : public UIElement {
 public:
  /**
   * @brief Constructs a Text element.
   *
   * @param x         The X position of the text.
   * @param y         The Y position of the text.
   * @param content   The initial text content.
   * @param font      The TTF_Font used to render the text.
   * @param color     The SDL_Color for the text.
   * @param renderer  The SDL_Renderer used for rendering the text.
   */
  Text(const int x, const int y, const std::string& content, TTF_Font* font,
       const SDL_Color& color, SDL_Renderer* renderer)
      : UIElement(x, y, 0, 0),
        content_(content),
        font_(font),
        color_(color),
        texture_(nullptr),
        renderer_(renderer) {
    if (!renderer_) {
      throw std::invalid_argument("[Text] SDL_Renderer is required to create a Text object.");
    }
    UpdateTexture();
  }

  /**
   * @brief Destructor to clean up the SDL_Texture.
   */
  ~Text() override { DestroyTexture(); }

  /**
   * @brief Sets the text content.
   *
   * Updates the texture if the content changes.
   *
   * @param new_content The new text content.
   */
  void SetContent(const std::string& new_content) {
    if (content_ != new_content) {
      content_ = new_content;
      UpdateTexture();
    }
  }

  /**
   * @brief Gets the current text content.
   *
   * @return The text content as a const reference.
   */
  const std::string& GetContent() const { return content_; }

  /**
   * @brief Sets the text color.
   *
   * Updates the texture if the color changes.
   *
   * @param new_color The new SDL_Color for the text.
   */
  void SetColor(const SDL_Color& new_color) {
    if (color_.r != new_color.r || color_.g != new_color.g ||
        color_.b != new_color.b || color_.a != new_color.a) {
      color_ = new_color;
      UpdateTexture();
    }
  }

  /**
   * @brief Sets the font.
   *
   * Updates the texture if the font changes.
   *
   * @param new_font The new TTF_Font.
   */
  void SetFont(TTF_Font* new_font) {
    if (font_ != new_font) {
      font_ = new_font;
      UpdateTexture();
    }
  }

  /**
   * @brief Gets the font used for rendering.
   *
   * @return The TTF_Font currently being used.
   */
  TTF_Font* GetFont() const { return font_; }

  /**
   * @brief Renders the text on the screen.
   *
   * @param renderer The SDL_Renderer used to draw the texture.
   */
  void Render(SDL_Renderer* renderer) override {
    if (!IsVisible() || !texture_) return;
    SDL_RenderCopy(renderer, texture_, nullptr, &GetBounds());
  }

  /**
   * @brief Handles input events (no-op for Text).
   *
   * @param event The SDL_Event to process.
   */
  void HandleInput(const SDL_Event& event) override {
    // No input handling for static text
  }

 private:
  std::string content_;  ///< The text content.
  TTF_Font* font_;       ///< The font used to render the text.
  SDL_Color color_;      ///< The color of the text.
  SDL_Texture* texture_; ///< The SDL_Texture for the rendered text.
  SDL_Renderer* renderer_; ///< The SDL_Renderer used for creating textures.

  /**
   * @brief Destroys the existing texture.
   */
  void DestroyTexture() {
    if (texture_) {
      SDL_DestroyTexture(texture_);
      texture_ = nullptr;
    }
  }

  /**
   * @brief Updates the texture with the current content, font, and color.
   */
  void UpdateTexture() {
    DestroyTexture();

    if (font_ && !content_.empty()) {
      if (SDL_Surface* text_surface =
              TTF_RenderText_Blended(font_, content_.c_str(), color_)) {
        texture_ = SDL_CreateTextureFromSurface(renderer_, text_surface);
        SetSize(text_surface->w, text_surface->h);  // Use SetSize to update bounds_
        SDL_FreeSurface(text_surface);
      } else {
        std::cerr << "[Text][ERROR] Failed to render text: " << TTF_GetError() << std::endl;
      }
    }
  }
};

#endif  // TEXT_HPP
