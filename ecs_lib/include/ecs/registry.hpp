#ifndef REGISTRY_HPP_
#define REGISTRY_HPP_

#include <unordered_map>
#include <typeindex>
#include <ranges>

#include "sparse_array.hpp"

namespace ecs {
template <typename... Components>
class registry {
public:
  using entity_t = std::size_t;

  using SparseArrayVariant = std::variant<SparseArray<Components>...>;

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

  entity_t spawn_entity() {
    if (!dead_entities_.empty()) {
      const entity_t id = dead_entities_.back();
      dead_entities_.pop_back();
      return id;
    }
    return next_entity_id_++;
  }

  /*
  void kill_entity(entity_t entity) {
    for (auto& [type, array] : components_arrays_) {
      try {
        if (type == std::type_index(typeid(position))) {
          auto& sparse_array = std::any_cast<SparseArray<position>&>(array);
          sparse_array.erase(entity);
          std::cout << "Killed component of type position for entity " << entity << std::endl;
        } else if (type == std::type_index(typeid(velocity))) {
          auto& sparse_array = std::any_cast<SparseArray<velocity>&>(array);
          sparse_array.erase(entity);
          std::cout << "Killed component of type velocity for entity " << entity << std::endl;
        }
      } catch (const std::bad_any_cast&) {
        std::cerr << "Failed to cast component of type " << type.name() << std::endl;
      }
    }
    dead_entities_.push_back(entity);
    std::cout << "Entity " << entity << " marked as dead." << std::endl;
  }
  */

  void kill_entity(entity_t entity) {
    for (auto& [type, array] : components_arrays_) {
      std::visit([&](auto& sparse_array) {
        if (entity < sparse_array.size()) {
          sparse_array.erase(entity);
          std::cout << "Killed component of type " << type.name() << " for entity " << entity << std::endl;
        }
      }, array);
    }
    dead_entities_.push_back(entity);
    std::cout << "Entity " << entity << " marked as dead." << std::endl;
  }

  template <typename Component>
    typename SparseArray<Component>::reference_type add_component(entity_t entity, const Component& component) {
    auto& components = get_components<Component>();
    return components.insert_at(entity, component);
  }

  template <typename Component, typename... Params>
  typename SparseArray<Component>::reference_type emplace_component(entity_t entity, Params&&... params) {
    auto& components = get_components<Component>();
    return components.emplace_at(entity, std::forward<Params>(params)...);
  }

  template <typename Component>
  void remove_component(entity_t entity) {
    auto& components = get_components<Component>();
    components.erase(entity);
  }

private:
  std::unordered_map<std::type_index, SparseArrayVariant> components_arrays_;
  //std::unordered_map<std::type_index, std::any> components_arrays_;
  std::vector<entity_t> dead_entities_;
  entity_t next_entity_id_ = 0;
};
}

#endif // REGISTRY_HPP_
