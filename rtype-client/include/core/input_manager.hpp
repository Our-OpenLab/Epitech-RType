#ifndef INPUT_MANAGER_HPP_
#define INPUT_MANAGER_HPP_

#include <SDL.h>

#include <chrono>
#include <cmath>
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <utility>

#include "core/player_actions.hpp"
#include "core/protocol.hpp"
#include "core/screen_manager.hpp"

namespace rtype {

class InputManager {
 public:
  struct PlayerInput {
    uint16_t actions;  ///< Encapsulates the player's actions (bitmask).
    float dir_x;       ///< Direction along the X-axis.
    float dir_y;       ///< Direction along the Y-axis.
  };

  using InputCallback = std::function<void(PlayerInput&&)>;

  /**
   * @brief Constructor for `InputManager`.
   *
   * @param callback Function called when player inputs are captured.
   * @param screen_manager Reference to the screen manager for converting
   * mouse coordinates.
   */
  explicit InputManager(InputCallback callback, ScreenManager& screen_manager)
      : callback_(std::move(callback)), screen_manager_(screen_manager) {}

  /**
   * @brief Handles SDL events related to player inputs.
   *
   * @param event The SDL event to process.
   */
  void HandleEvent(const SDL_Event& event) {
    static constexpr auto keycode_to_action = [] {
      std::array<uint16_t, SDLK_z + 1> temp{};
      temp['z'] = static_cast<uint16_t>(PlayerAction::MoveUp);
      temp['s'] = static_cast<uint16_t>(PlayerAction::MoveDown);
      temp['q'] = static_cast<uint16_t>(PlayerAction::MoveLeft);
      temp['d'] = static_cast<uint16_t>(PlayerAction::MoveRight);
      temp[' '] = static_cast<uint16_t>(PlayerAction::Shoot);
      temp[SDLK_a] = static_cast<uint16_t>(PlayerAction::AutoShoot);
      return temp;
    }();

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

      mouse_position_ =
          screen_manager_.MouseToWorldCoordinates(mouse_x, mouse_y);
    }

    if (actions_changed || event.type == SDL_MOUSEMOTION) {
      PlayerInput input{current_actions_, mouse_position_.first - 0.5f,
                        mouse_position_.second - 0.5f};

      callback_(std::move(input));
    }
  }

 private:
  InputCallback callback_;  ///< Function called when player inputs are captured.
  ScreenManager& screen_manager_;  ///< Reference to the screen manager.
  uint16_t current_actions_{};     ///< Current player actions (bitmask).
  std::pair<float, float>
      mouse_position_{};  ///< Current mouse position in world coordinates.
};

}  // namespace rtype

#endif  // INPUT_MANAGER_HPP_
