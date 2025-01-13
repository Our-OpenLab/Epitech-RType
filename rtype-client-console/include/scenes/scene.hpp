#ifndef SCENE_HPP
#define SCENE_HPP

class Scene {
public:
  virtual ~Scene() = default;

  // Called when the scene becomes active
  virtual void Enter() = 0;

  // Called when the scene is no longer active
  virtual void Exit() = 0;

  // Update the scene logic
  virtual void Update(float deltaTime) = 0;

  // Render the scene
  virtual void Render() = 0;

  // Handle user input
  virtual void HandleInput(const SDL_Event& e) = 0;
};

#endif // SCENE_HPP
