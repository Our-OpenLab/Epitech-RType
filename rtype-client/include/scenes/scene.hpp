#ifndef SCENE_HPP
#define SCENE_HPP

#include <SDL2/SDL.h>

#include "core/renderer.hpp"
#include "core/event_queue.hpp"

namespace rtype {

/**
 * @brief Abstract base class for all scenes.
 */
class Scene {
public:
  explicit Scene() = default;
  virtual ~Scene() = default;

  virtual void Enter() = 0;          ///< Called when entering the scene.
  virtual void Exit() = 0;           ///< Called when exiting the scene.
  virtual void Update(float delta_time) = 0; ///< Update scene logic.
  virtual void Render() = 0;         ///< Render the scene.
  virtual void HandleInput(const SDL_Event& event) = 0; ///< Handle user input.
};

} // namespace rtype

#endif  // SCENE_HPP
