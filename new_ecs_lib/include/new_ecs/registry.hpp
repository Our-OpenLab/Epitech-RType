#ifndef REGISTRY_HPP_
#define REGISTRY_HPP_

#include <functional>
#include <queue>
#include <typeindex>
#include <unordered_map>
#include <variant>
#include <vector>
#include <iostream>

#include "sparse_array.hpp"
#include "zipper.hpp"

namespace ecs {

template <typename T, typename... Ts>
struct TypeIndex;

template <typename T, typename... Ts>
struct TypeIndex<T, T, Ts...> {
  static constexpr size_t value = 0;
};

template <typename T, typename U, typename... Ts>
struct TypeIndex<T, U, Ts...> {
  static constexpr size_t value = 1 + TypeIndex<T, Ts...>::value;
};

template <typename... Components>
class Registry final {
 public:
  using entity_t = std::size_t;
  using SparseArrayVariant = std::variant<SparseArray<Components>...>;

  template <typename Component>
  SparseArray<Component>& register_component() {
    auto [it, inserted] = components_arrays_.emplace(
        std::type_index(typeid(Component)), SparseArray<Component>{});
    ensure_signature_size();
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

  entity_t spawn_entity() {
    entity_t id;
    if (!dead_entities_.empty()) {
      id = dead_entities_.top();
      dead_entities_.pop();
    } else {
      id = next_entity_id_++;
    }
    ensure_signature_size();
    return id;
  }

  void kill_entity(entity_t entity) {
    for (auto& [type, array] : components_arrays_) {
      std::visit(
          [&](auto& sparse_array) {
            if (entity < sparse_array.size()) {
              sparse_array.erase(entity);
            }
          },
          array);
    }
    entity_signatures_[entity] = 0;  // Clear the entity's signature
    dead_entities_.push(entity);
  }

  template <typename Component>
  typename SparseArray<Component>::value_type& add_component(
      entity_t entity, Component&& component) {
    auto& components = get_components<Component>();
    ensure_signature_size(entity);
    update_signature<Component>(entity);
    return components.insert_at(entity, std::forward<Component>(component));
  }

  template <typename Component>
  void remove_component(entity_t entity) {
    auto& components = get_components<Component>();
    if (entity < components.size()) {
      components.erase(entity);
      clear_signature<Component>(entity);
    }
  }

  template <typename Component>
  [[nodiscard]] bool has_component(entity_t entity) const {
    const auto& components = get_components<Component>();
    return components.is_valid(entity);
  }

  template <typename... QueryComponents>
  [[nodiscard]] std::vector<entity_t> get_entities_with_components() const {
    static const uint64_t required_mask =
        ((1ULL << TypeIndex<QueryComponents, Components...>::value) | ...);
    std::vector<entity_t> entities;

    for (entity_t i = 0; i < entity_signatures_.size(); ++i) {
      //if ((entity_signatures_[i] & required_mask) == required_mask) {
      //  entities.push_back(i);
      //}
      if ((entity_signatures_[i] & required_mask) == required_mask) {
        entities.push_back(i);
        std::cout << "[DEBUG] Entity " << i << " matches required components.\n";
      } else {
        std::cout << "[DEBUG] Entity " << i << " does NOT match required components.\n";
      }
    }

    return entities;
  }

  template <typename... QueryComponents>
auto get_filtered_zipper() {
    auto entities = get_entities_with_components<QueryComponents...>();

    // Log des entités transmises au Zipper
    std::cout << "[DEBUG] Entities passed to Zipper: ";
    for (const auto& entity : entities) {
      std::cout << entity << " ";
    }
    std::cout << "\n";

    // Retourne le Zipper avec les références des containers
    return ecs::Zipper(entities, (get_components<QueryComponents>())...);
  }


  template <typename Function>
  void add_system(Function&& system) {
    systems_.emplace_back([=, this]() { system(*this); });
  }

  void run_systems() const {
    for (const auto& system : systems_) {
      system();
    }
  }

 private:
  std::vector<std::function<void()>> systems_;
  std::unordered_map<std::type_index, SparseArrayVariant> components_arrays_;
  std::priority_queue<entity_t, std::vector<entity_t>, std::greater<>> dead_entities_;
  entity_t next_entity_id_ = 0;

  // Signatures for all entities
  std::vector<uint64_t> entity_signatures_;

  // Helper: Ensure entity_signatures_ is large enough
  void ensure_signature_size(entity_t entity = 0) {
    if (entity >= entity_signatures_.size()) {
      entity_signatures_.resize(entity + 1, 0);
    }
  }

  // Update the signature of an entity to include a component
  template <typename Component>
  void update_signature(entity_t entity) {
    static const uint64_t component_mask = 1ULL << TypeIndex<Component, Components...>::value;
    entity_signatures_[entity] |= component_mask;
  }

  // Clear the component bit in the entity's signature
  template <typename Component>
  void clear_signature(entity_t entity) {
    static const uint64_t component_mask = 1ULL << TypeIndex<Component, Components...>::value;
    entity_signatures_[entity] &= ~component_mask;
  }
};
}  // namespace ecs

#endif  // REGISTRY_HPP_
