#ifndef LOGIN_SCENE_HPP
#define LOGIN_SCENE_HPP

#include <memory>
#include <string>
#include <SDL2/SDL_ttf.h>

#include "scene.hpp"
#include "ui/text_box.hpp"
#include "ui/button.hpp"
#include "ui/text.hpp"
#include "core/renderer.hpp"

/**
 * @brief A Scene where the user can type in a TextBox and click a Button to "login".
 */
class LoginScene : public Scene {
public:
    LoginScene(Renderer& renderer, TTF_Font* font)
        : renderer(renderer)
        , font(font)
        , titleText(nullptr)
        , usernameBox(nullptr)
        , loginButton(nullptr)
        , isCompleted(false)
    {
        // Example color (black) for text
        SDL_Color blackColor {0, 0, 0, 255};

        // Title text
        titleText = std::make_unique<Text>(
            /* x = */ 100,
            /* y = */ 40,
            /* content = */ "Please Login:",
            font,
            blackColor,
            renderer.GetSDLRenderer()
        );

        // A text box for the username
        usernameBox = std::make_unique<TextBox>(
            /* x = */ 100,
            /* y = */ 100,
            /* width = */ 300,
            /* height= */ 40,
            font,
            blackColor,
            renderer.GetSDLRenderer()
        );

        // Create a button at (100, 160), sized 100x40, with multi-states
        loginButton = std::make_unique<Button>(
            /* x = */ 100,
            /* y = */ 160,
            /* width = */ 100,
            /* height= */ 40
        );

        // Optionally: set textures for different states (Normal, Hover, Pressed, Disabled).
        // For now, we might have them as nullptr or load them from somewhere.
        // e.g.:
        //   SDL_Texture* normalTex = LoadTexture("button_normal.png");
        //   SDL_Texture* hoverTex  = LoadTexture("button_hover.png");
        //   SDL_Texture* pressedTex= LoadTexture("button_pressed.png");
        //   ...
        // loginButton->SetTexture(ButtonState::Normal, normalTex);
        // loginButton->SetTexture(ButtonState::Hover, hoverTex);
        // loginButton->SetTexture(ButtonState::Pressed, pressedTex);
        // ...
        // If you skip these calls, the Button's fallback rectangle rendering will appear.

        // Define the button's onClick behavior
        loginButton->SetOnClick([this]() {
            OnLoginButtonClicked();
        });
    }

    ~LoginScene() override = default;

    //----------------------------
    // Scene interface
    //----------------------------

    void Enter() override {
        std::cout << "[LoginScene] Enter()\n";
        // Possibly activate text input
        SDL_StartTextInput();
    }

    void Exit() override {
        std::cout << "[LoginScene] Exit()\n";
        // Possibly deactivate text input
        SDL_StopTextInput();
    }

    void Update(float deltaTime) override {
        (void)deltaTime; // not used, but here if you want animations
        // If you have animations or want to poll the button's state in detail,
        // you could do: loginButton->Update(deltaTime), etc.
    }

    void Render() override {
        // If you do NOT clear the screen externally, you can do it here:
        // renderer.SetDrawColor(255, 255, 255, 255); // white
        // renderer.Clear();

        // Title
        if (titleText) {
            titleText->Render(renderer.GetSDLRenderer());
        }

        // Textbox
        if (usernameBox) {
            usernameBox->Render(renderer.GetSDLRenderer());
        }

        // Button
        if (loginButton) {
            // If there's no texture assigned for the current state,
            // we'll do a fallback rectangle for clarity:
            // The Button itself won't do the fallback. We can add a quick example:

            //if (!loginButton->HasTextureForCurrentState()) {
            //    SDL_SetRenderDrawColor(renderer.GetSDLRenderer(), 128, 128, 128, 255);
            //    const SDL_Rect buttonRect = loginButton->GetBounds();
            //    SDL_RenderFillRect(renderer.GetSDLRenderer(), &buttonRect);

                // Outline
            //    SDL_SetRenderDrawColor(renderer.GetSDLRenderer(), 0, 0, 0, 255);
            //    SDL_RenderDrawRect(renderer.GetSDLRenderer(), &buttonRect);
            //}

            loginButton->Render(renderer.GetSDLRenderer());
        }

        // If you want, you can call renderer.Present() here or in your main loop
    }

    void HandleInput(const SDL_Event& e) override {
        //SDL_Event e;
        //while (SDL_PollEvent(&e)) {
        //    if (e.type == SDL_QUIT) {
                // Let's mark the scene as completed so the app can quit or pop scene
       //         isCompleted = true;
       //         return;
       //     }

            // Pass events to text box
            if (usernameBox) {
                usernameBox->HandleInput(e);
            }

            // Pass events to button
            if (loginButton) {
                loginButton->HandleInput(e);
            }
      //  }
    }

    //----------------------------
    // Additional logic
    //----------------------------

    bool IsCompleted() const {
        return isCompleted;
    }

private:
    void OnLoginButtonClicked() {
        std::cout << "[LoginScene] Login button clicked!\n";

        if (usernameBox) {
            std::string enteredText = usernameBox->GetContent();
            std::cout << "You typed: " << enteredText << "\n";
            // Real "login" logic could go here, or you could just
            // pop/replace the scene, etc.

            // Example: Mark the scene as completed => SceneManager could pop it
            isCompleted = true;
        }
    }

private:
    Renderer& renderer;
    TTF_Font* font;

    // UI Elements
    std::unique_ptr<Text>     titleText;
    std::unique_ptr<TextBox>  usernameBox;
    std::unique_ptr<Button>   loginButton;

    bool isCompleted;
};

#endif // LOGIN_SCENE_HPP
