#ifndef SCENE_MANAGER_HPP
#define SCENE_MANAGER_HPP

#include <stack>
#include <memory>
#include <stdexcept>
#include <iostream>

#include "scene.hpp"

class SceneManager {
public:
  explicit SceneManager(std::unique_ptr<Scene> initialScene) {
    if (!initialScene) {
      throw std::invalid_argument("[SceneManager] Initial scene cannot be null.");
    }
    sceneStack.push(std::move(initialScene));
    GetActiveScene().Enter();
  }

  ~SceneManager() {
    while (!sceneStack.empty()) {
      GetActiveScene().Exit();
      sceneStack.pop();
    }
  }

  void PushScene(std::unique_ptr<Scene> scene) {
    if (!scene) {
      throw std::invalid_argument("[SceneManager] Cannot push a null scene.");
    }
    GetActiveScene().Exit();
    sceneStack.push(std::move(scene));
    GetActiveScene().Enter();
  }

  void PopScene() {
    if (sceneStack.size() <= 1) {
      std::cerr << "[SceneManager] Cannot pop the last scene." << std::endl;
      return;
    }
    GetActiveScene().Exit();
    sceneStack.pop();
    GetActiveScene().Enter();
  }

  void ReplaceScene(std::unique_ptr<Scene> scene) {
    if (!scene) {
      throw std::invalid_argument("[SceneManager] Cannot replace with a null scene.");
    }
    GetActiveScene().Exit();
    sceneStack.pop();
    sceneStack.push(std::move(scene));
    GetActiveScene().Enter();
  }

  Scene& GetActiveScene() const {
    return *sceneStack.top();
  }

  void Update(float deltaTime) {
    GetActiveScene().Update(deltaTime);
  }

  void Render() {
    GetActiveScene().Render();
  }

  void HandleInput(const SDL_Event& e) {
    GetActiveScene().HandleInput(e);
  }

private:
  std::stack<std::unique_ptr<Scene>> sceneStack;
};

#endif // SCENE_MANAGER_HPP
