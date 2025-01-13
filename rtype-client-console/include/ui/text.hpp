#ifndef TEXT_HPP
#define TEXT_HPP

#include <SDL2/SDL_ttf.h>
#include <string>
#include "ui_element.hpp"

class Text : public UIElement {
public:
  Text(const int x, const int y, const std::string& content, TTF_Font* font,
      const SDL_Color color, SDL_Renderer* renderer)
      : UIElement(x, y, 0, 0), content(content), font(font), color(color), texture(nullptr), renderer(renderer) {
    UpdateTexture();
  }

  ~Text() override {
    DestroyTexture();
  }

  void SetContent(const std::string& newContent) {
    if (content != newContent) {
      content = newContent;
      UpdateTexture();
    }
  }

  const std::string& GetContent() const {
    return content;
  }

  void SetColor(const SDL_Color& newColor) {
    if (color.r != newColor.r || color.g != newColor.g ||
        color.b != newColor.b || color.a != newColor.a) {
      color = newColor;
      UpdateTexture();
    }
  }

  void SetFont(TTF_Font* newFont) {
    if (font != newFont) {
      font = newFont;
      UpdateTexture();
    }
  }

  void Render(SDL_Renderer* renderer) override {
    if (!IsVisible() || !texture) return;
    SDL_RenderCopy(renderer, texture, nullptr, &bounds);
  }

  void HandleInput(const SDL_Event&) override {
    // No input handling for static text
  }

private:
  std::string content;
  TTF_Font* font;
  SDL_Color color;
  SDL_Texture* texture;
  SDL_Renderer* renderer;

  void DestroyTexture() {
    if (texture) {
      SDL_DestroyTexture(texture);
      texture = nullptr;
    }
  }

  void UpdateTexture() {
    DestroyTexture();

    if (font && !content.empty()) {
      if (SDL_Surface* textSurface =
              TTF_RenderText_Blended(font, content.c_str(), color)) {
        texture = SDL_CreateTextureFromSurface(renderer, textSurface);
        bounds.w = textSurface->w;
        bounds.h = textSurface->h;
        SDL_FreeSurface(textSurface);
      } else {
        std::cerr << "[Text][ERROR] Failed to render text: " << TTF_GetError() << std::endl;
      }
    }
  }
};

#endif // TEXT_HPP
