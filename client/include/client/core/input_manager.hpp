#ifndef INPUT_MANAGER_HPP_
#define INPUT_MANAGER_HPP_

#include <SDL.h>

#include <cstdint>
#include <functional>
#include <shared/player_actions.hpp>

#include "client/core/screen_manager.hpp"

class InputManager {
public:
    struct PlayerInput {
        uint16_t actions;
        float dir_x;
        float dir_y;
        uint32_t timestamp;
    };

    using InputCallback = std::function<void(PlayerInput&&)>;

    explicit InputManager(InputCallback callback, ScreenManager& screen_manager)
        : callback_(std::move(callback)), screen_manager_(screen_manager) {}

    void HandleEvent(const SDL_Event& event,
    const std::chrono::steady_clock::time_point current_time) {
    static constexpr uint16_t keycode_to_action[SDLK_z + 1] = {
        ['z'] = static_cast<uint16_t>(PlayerAction::MoveUp),
        ['s'] = static_cast<uint16_t>(PlayerAction::MoveDown),
        ['q'] = static_cast<uint16_t>(PlayerAction::MoveLeft),
        ['d'] = static_cast<uint16_t>(PlayerAction::MoveRight),
        [' '] = static_cast<uint16_t>(PlayerAction::Shoot),
    };

    bool actions_changed = false;
    int mouse_x = 0, mouse_y = 0;

    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        const bool key_down = (event.type == SDL_KEYDOWN);

        if (const SDL_Keycode key = event.key.keysym.sym;
            key >= 0 && key <= SDLK_z && keycode_to_action[key] != 0) {
            const uint16_t action = keycode_to_action[key];
            const uint16_t previous_actions = current_actions_;

            if (key_down) {
                current_actions_ |= action;
            } else {
                current_actions_ &= ~action;
            }

            actions_changed = (current_actions_ != previous_actions);
        }
    } else if (event.type == SDL_MOUSEMOTION) {
        SDL_GetMouseState(&mouse_x, &mouse_y);

        mouse_position_ = screen_manager_.MouseToWorldCoordinates(
            mouse_x, mouse_y);
    }

    if (actions_changed || event.type == SDL_MOUSEMOTION) {
        const auto timestamp =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                current_time.time_since_epoch())
                .count();

        std::cout << "Mouse position: " << mouse_position_.first << ", "
                  << mouse_position_.second << '\n';

        PlayerInput input{current_actions_,
                          mouse_position_.first - 0.5f, mouse_position_.second - 0.5f,
                          static_cast<uint32_t>(timestamp)};

        callback_(std::move(input));
    }
}

private:
    InputCallback callback_;
    ScreenManager& screen_manager_;
    uint16_t current_actions_{};
    std::pair<float, float> mouse_position_{};
};

#endif  // INPUT_MANAGER_HPP_
