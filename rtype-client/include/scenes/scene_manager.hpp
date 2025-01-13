#ifndef SCENE_MANAGER_HPP
#define SCENE_MANAGER_HPP

#include <stack>
#include <memory>
#include <stdexcept>
#include <iostream>

#include "scene.hpp"

namespace rtype {

class SceneManager {
public:
  SceneManager() = default;

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
    if (!sceneStack.empty()) {
      GetActiveScene().Exit();
    }
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
    if (!sceneStack.empty()) {
      GetActiveScene().Exit();
      sceneStack.pop();
    }
    sceneStack.push(std::move(scene));
    GetActiveScene().Enter();
  }

  Scene& GetActiveScene() const {
    if (sceneStack.empty()) {
      throw std::runtime_error("[SceneManager] No active scene.");
    }
    return *sceneStack.top();
  }

  void Update(float deltaTime) {
    if (!sceneStack.empty()) {
      GetActiveScene().Update(deltaTime);
    }
  }

  void Render() {
    if (!sceneStack.empty()) {
      GetActiveScene().Render();
    }
  }

  void HandleInput(const SDL_Event& e) {
    if (!sceneStack.empty()) {
      GetActiveScene().HandleInput(e);
    }
  }

private:
  std::stack<std::unique_ptr<Scene>> sceneStack;
};

}

#endif // SCENE_MANAGER_HPP
