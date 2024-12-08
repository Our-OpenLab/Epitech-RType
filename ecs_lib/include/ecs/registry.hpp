#ifndef REGISTRY_HPP_
#define REGISTRY_HPP_

#include <any>
#include <chrono>
#include <functional>
#include <queue>
#include <ranges>
#include <typeindex>
#include <unordered_map>

#include "sparse_array.hpp"

namespace ecs {
template <typename... Components>
class Registry final {
public:
  using entity_t = std::size_t;

  using SparseArrayVariant = std::variant<SparseArray<Components>...>;
  using SystemFunction = std::function<void(Registry&, float, std::chrono::milliseconds)>;

  template <typename Component>
  SparseArray<Component>& register_component() {
    auto [it, inserted] = components_arrays_.emplace(
        std::type_index(typeid(Component)),
        SparseArray<Component>{});
    return std::get<SparseArray<Component>>(it->second);
  }

  template <typename Component>
  SparseArray<Component>& get_components() {
    return std::get<SparseArray<Component>>(
        components_arrays_.at(std::type_index(typeid(Component))));
  }

  template <typename Component>
  const SparseArray<Component>& get_components() const {
    return std::get<SparseArray<Component>>(
        components_arrays_.at(std::type_index(typeid(Component))));
  }

  template <typename Component>
  SparseArray<Component>& get_components_cached() {
    static SparseArray<Component>* cache = nullptr;
    if (!cache) {
      cache = &std::get<SparseArray<Component>>(
          components_arrays_.at(std::type_index(typeid(Component))));
    }
    return *cache;
  }

  template <typename Component>
  const SparseArray<Component>& get_components_cached() const {
    static const SparseArray<Component>* cache = nullptr;
    if (!cache) {
      cache = &std::get<SparseArray<Component>>(
          components_arrays_.at(std::type_index(typeid(Component))));
    }
    return *cache;
  }

  entity_t spawn_entity() {
    if (!dead_entities_.empty()) {
      //const entity_t id = dead_entities_.back();
      //dead_entities_.pop_back();
      const entity_t id = dead_entities_.top();
      dead_entities_.pop();
      return id;
    }
    return next_entity_id_++;
  }

  void kill_entity(const entity_t& entity) {
    for (auto& [type, array] : components_arrays_) {
      std::visit([&](auto& sparse_array) {
        if (entity < sparse_array.size()) {
          sparse_array.erase(entity);
        }
      }, array);
    }
    //dead_entities_.push_back(entity);
    dead_entities_.push(entity);
  }

  template <typename Component>
  typename SparseArray<Component>::reference_type add_component(
    const entity_t& entity, Component&& component) {
    auto& components = get_components_cached<Component>();
    return components.insert_at(entity, std::forward<Component>(component));
  }

  template <typename Component, typename... Params>
  typename SparseArray<Component>::reference_type emplace_component(
      const entity_t& entity, Params&&... params) {
    auto& components = get_components<Component>();
    return components.emplace_at(entity, std::forward<Params>(params)...);
  }

  template <typename Component>
  void remove_component(entity_t entity) {
    auto& components = get_components<Component>();
    components.erase(entity);
  }

  template <typename Func>
    void add_system(Func&& func) {
    systems_.emplace_back(std::forward<Func>(func));
  }

  void run_systems(const float delta_time, const std::chrono::milliseconds render_time) {
    for (auto& system : systems_) {
      system(*this, delta_time, render_time);
    }
  }



/*
  void run_systems() const {
    std::vector<std::future<void>> futures;
    for (auto& system : systems_) {
      futures.push_back(std::async(std::launch::async, system));
    }
    for (auto& future : futures) {
      future.get();
    }
  }
  */

private:
  std::vector<SystemFunction> systems_;

  std::unordered_map<std::type_index, SparseArrayVariant> components_arrays_;
  std::priority_queue<entity_t, std::vector<entity_t>, std::greater<>> dead_entities_;
  entity_t next_entity_id_ = 0;
};
}

#endif // REGISTRY_HPP_
