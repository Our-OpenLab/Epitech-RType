#ifndef TEXT_BOX_HPP
#define TEXT_BOX_HPP

#include "ui_element.hpp"
#include "text.hpp"

class TextBox : public UIElement {
public:
  TextBox(const int x, const int y, const int width, const int height,
        TTF_Font* font,
        SDL_Color textColor,
        SDL_Renderer* renderer)
    : UIElement(x, y, width, height)
    , textObj(std::make_unique<Text>(x + padding, y + padding, content, font,
                                      textColor, renderer))
    , font(font)
    , textColor(textColor)
    , renderer(renderer)
    , content("")
    , isFocused(false)
  {}

  ~TextBox() override = default;

  void Render(SDL_Renderer* renderer) override {
    if (!IsVisible()) return;

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    const SDL_Rect backgroundRect = GetBounds();
    SDL_RenderFillRect(renderer, &backgroundRect);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &backgroundRect);

    if (textObj) {
      textObj->Render(renderer);
    }
  }

  void HandleInput(const SDL_Event& event) override {
    if (!IsEnabled() || !IsVisible()) {
      return;
    }

    switch (event.type) {
      case SDL_MOUSEBUTTONDOWN: {
        const int mouseX = event.button.x;
        const int mouseY = event.button.y;
        isFocused = IsPointInside(mouseX, mouseY);
        break;
      }
      case SDL_TEXTINPUT: {
        if (isFocused) {
          content += event.text.text;
          UpdateTextObj();
        }
        break;
      }
      case SDL_KEYDOWN: {
        if (isFocused) {
          if (event.key.keysym.sym == SDLK_BACKSPACE) {
            if (!content.empty()) {
              content.pop_back();
              UpdateTextObj();
            }
          }
          else if (event.key.keysym.sym == SDLK_RETURN) {
            // In a real text box, you might do something else,
            // like losing focus, or sending the content to the server
            isFocused = false;
          }
        }
        break;
      }
      default:
        break;
    }
  }

  void Update(float /*deltaTime*/) override {}

  void SetContent(const std::string& newContent) {
    content = newContent;
    UpdateTextObj();
  }

  const std::string& GetContent() const {
    return content;
  }

  bool HasFocus() const {
    return isFocused;
  }

  void UpdateTextObj() const {
    textObj->SetContent(content);
  }

private:
  std::unique_ptr<Text> textObj;

  TTF_Font* font;
  SDL_Color textColor;
  SDL_Renderer* renderer;

  std::string content;
  bool isFocused;

  static constexpr int padding = 4;
};

#endif // TEXT_BOX_HPP
