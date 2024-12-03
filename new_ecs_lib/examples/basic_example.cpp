#include <iostream>
#include "new_ecs/registry.hpp"

struct position {
    float x, y;
};

struct velocity {
    float vx, vy;
};

// Fonction utilitaire pour afficher l'état actuel du registre
void log_status(const ecs::Registry<position, velocity>& reg, const std::string& message) {
    std::cout << "[LOG] " << message << "\n";
    const auto entities_with_position = reg.get_entities_with_components<position>();
    const auto entities_with_velocity = reg.get_entities_with_components<velocity>();

    std::cout << "Entities with position: ";
    for (const auto& e : entities_with_position) {
        std::cout << e << " ";
    }
    std::cout << "\n";

    std::cout << "Entities with velocity: ";
    for (const auto& e : entities_with_velocity) {
        std::cout << e << " ";
    }
    std::cout << "\n";
}

// Test de base pour ajouter et manipuler des composants
void test_basic_operations() {
    ecs::Registry<position, velocity> reg;

    // Enregistrement des composants
    reg.register_component<position>();
    reg.register_component<velocity>();
    log_status(reg, "After registering components");

    // Création d'entités et ajout de composants
    auto entity1 = reg.spawn_entity();
    reg.add_component(entity1, position{0.0f, 0.0f});
    reg.add_component(entity1, velocity{1.0f, 1.5f});

    auto entity2 = reg.spawn_entity();
    reg.add_component(entity2, position{10.0f, 10.0f});
    log_status(reg, "After adding components to entity1 and entity2");

    // Mise à jour d'un composant
    auto& positions = reg.get_components<position>();
    auto& velocities = reg.get_components<velocity>();
    positions[entity1].x += velocities[entity1].vx;
    positions[entity1].y += velocities[entity1].vy;

    std::cout << "[TEST] Updated entity1 position to: ("
              << positions[entity1].x << ", " << positions[entity1].y << ")\n";

    // Suppression d'une entité
    reg.kill_entity(entity1);
    log_status(reg, "After killing entity1");
}

// Test de suppression de composants spécifiques
void test_component_removal() {
    ecs::Registry<position, velocity> reg;

    // Enregistrement des composants
    reg.register_component<position>();
    reg.register_component<velocity>();

    // Création et configuration d'entités
    auto entity1 = reg.spawn_entity();
    reg.add_component(entity1, position{5.0f, 5.0f});
    reg.add_component(entity1, velocity{2.0f, 3.0f});

    log_status(reg, "Before removing velocity from entity1");

    // Suppression d'un composant
    reg.remove_component<velocity>(entity1);
    log_status(reg, "After removing velocity from entity1");

    auto& positions = reg.get_components<position>();
    if (positions.is_valid(entity1)) {
        std::cout << "[SUCCESS] Entity1 position remains: ("
                  << positions[entity1].x << ", " << positions[entity1].y << ")\n";
    } else {
        std::cout << "[ERROR] Entity1 position was unexpectedly removed.\n";
    }

    auto& velocities = reg.get_components<velocity>();
    if (!velocities.is_valid(entity1)) {
        std::cout << "[SUCCESS] Entity1 velocity has been removed.\n";
    } else {
        std::cout << "[ERROR] Entity1 velocity was not removed as expected.\n";
    }
}

// Test de réutilisation d'entités supprimées
void test_entity_reuse() {
    ecs::Registry<position, velocity> reg;

    // Enregistrement des composants
    reg.register_component<position>();

    // Création et suppression d'une entité
    auto entity1 = reg.spawn_entity();
    reg.add_component(entity1, position{10.0f, 10.0f});
    reg.kill_entity(entity1);
    log_status(reg, "After killing entity1");

    // Réutilisation de l'entité
    auto reused_entity = reg.spawn_entity();
    reg.add_component(reused_entity, position{20.0f, 20.0f});
    log_status(reg, "After reusing a dead entity ID");

    auto& positions = reg.get_components<position>();
    if (positions.is_valid(reused_entity)) {
        std::cout << "[SUCCESS] Reused entity has position: ("
                  << positions[reused_entity].x << ", " << positions[reused_entity].y << ")\n";
    }
}

// Test d'exécution de systèmes sur des entités filtrées
void test_system_execution() {
    ecs::Registry<position, velocity> reg;

    // Enregistrement des composants
    reg.register_component<position>();
    reg.register_component<velocity>();

    // Création d'entités
    auto entity1 = reg.spawn_entity();
    reg.add_component(entity1, position{0.0f, 0.0f});
    reg.add_component(entity1, velocity{1.0f, 1.0f});

    auto entity2 = reg.spawn_entity();
    reg.add_component(entity2, position{10.0f, 10.0f});  // Pas de velocity pour entity2

    log_status(reg, "Before running systems");

    // Ajout et exécution d'un système
    reg.add_system([](ecs::Registry<position, velocity>& reg) {
        auto zipper = reg.get_filtered_zipper<position, velocity>();
        for (auto&& [pos, vel] : zipper) {
            std::cout << "[SYSTEM] Updating entity: "
                      << "Position before: (" << pos.x << ", " << pos.y << "), "
                      << "Velocity: (" << vel.vx << ", " << vel.vy << ")\n";
            pos.x += vel.vx;
            pos.y += vel.vy;
            std::cout << "[SYSTEM] Position after: (" << pos.x << ", " << pos.y << ")\n";
        }
    });

    reg.run_systems();
    log_status(reg, "After running systems");

    // Validation des mises à jour
    auto& positions = reg.get_components<position>();
    auto& velocities = reg.get_components<velocity>();

    if (positions.is_valid(entity1) && velocities.is_valid(entity1)) {
        std::cout << "[TEST] Entity1 new position: ("
                  << positions[entity1].x << ", " << positions[entity1].y << ")\n";
        if (positions[entity1].x == 1.0f && positions[entity1].y == 1.0f) {
            std::cout << "[SUCCESS] Entity1 position updated correctly.\n";
        } else {
            std::cout << "[ERROR] Entity1 position update incorrect.\n";
        }
    }

    if (positions.is_valid(entity2)) {
        std::cout << "[TEST] Entity2 position remains unchanged: ("
                  << positions[entity2].x << ", " << positions[entity2].y << ")\n";
        if (positions[entity2].x == 10.0f && positions[entity2].y == 10.0f) {
            std::cout << "[SUCCESS] Entity2 position is unchanged as expected.\n";
        } else {
            std::cout << "[ERROR] Entity2 position should not have changed.\n";
        }
    }
}

void test_get_entities_with_components() {
    ecs::Registry<position, velocity> reg;

    // Enregistrement des composants
    reg.register_component<position>();
    reg.register_component<velocity>();

    // Création et ajout de composants à différentes entités
    auto entity1 = reg.spawn_entity();
    reg.add_component(entity1, position{0.0f, 0.0f});
    reg.add_component(entity1, velocity{1.0f, 1.0f});

    auto entity2 = reg.spawn_entity();
    reg.add_component(entity2, position{10.0f, 10.0f});  // Pas de velocity

    auto entity3 = reg.spawn_entity();
    reg.add_component(entity3, velocity{-1.0f, -1.0f});  // Pas de position

    auto entity4 = reg.spawn_entity();  // Aucun composant

    log_status(reg, "Before testing get_entities_with_components");

    // Test : Récupérer les entités avec position
    auto entities_with_position = reg.get_entities_with_components<position>();
    std::cout << "[TEST] Entities with position: ";
    for (auto e : entities_with_position) {
        std::cout << e << " ";
    }
    std::cout << "\n";
    if (entities_with_position == std::vector<ecs::Registry<>::entity_t>{entity1, entity2}) {
        std::cout << "[SUCCESS] Entities with position correctly identified.\n";
    } else {
        std::cout << "[ERROR] Incorrect entities returned for position.\n";
    }

    // Test : Récupérer les entités avec velocity
    auto entities_with_velocity = reg.get_entities_with_components<velocity>();
    std::cout << "[TEST] Entities with velocity: ";
    for (auto e : entities_with_velocity) {
        std::cout << e << " ";
    }
    std::cout << "\n";
    if (entities_with_velocity == std::vector<ecs::Registry<>::entity_t>{entity1, entity3}) {
        std::cout << "[SUCCESS] Entities with velocity correctly identified.\n";
    } else {
        std::cout << "[ERROR] Incorrect entities returned for velocity.\n";
    }

    // Test : Récupérer les entités avec position ET velocity
    auto entities_with_both = reg.get_entities_with_components<position, velocity>();
    std::cout << "[TEST] Entities with both position and velocity: ";
    for (auto e : entities_with_both) {
        std::cout << e << " ";
    }
    std::cout << "\n";
    if (entities_with_both == std::vector<ecs::Registry<>::entity_t>{entity1}) {
        std::cout << "[SUCCESS] Entities with both components correctly identified.\n";
    } else {
        std::cout << "[ERROR] Incorrect entities returned for position and velocity.\n";
    }
}

void test_filtered_zipper() {
  ecs::Registry<position, velocity> reg;

  // Enregistrement des composants
  reg.register_component<position>();
  reg.register_component<velocity>();

  // Création d'entités avec différentes combinaisons de composants
  auto entity1 = reg.spawn_entity();
  reg.add_component(entity1, position{0.0f, 0.0f});
  reg.add_component(entity1, velocity{1.0f, 1.0f}); // position + velocity

  auto entity2 = reg.spawn_entity();
  reg.add_component(entity2, position{10.0f, 10.0f}); // uniquement position

  auto entity3 = reg.spawn_entity();
  reg.add_component(entity3, velocity{-1.0f, -1.0f}); // uniquement velocity

  auto entity4 = reg.spawn_entity(); // Aucun composant

  log_status(reg, "Before testing get_filtered_zipper");

  try {
    auto zipper = reg.get_filtered_zipper<position, velocity>();
    size_t entity_count = 0;

    for (auto&& [pos, vel] : zipper) {
      std::cout << "Entity " << entity_count++ << ": Position before update: ("
                << pos.x << ", " << pos.y << "), Velocity: ("
                << vel.vx << ", " << vel.vy << ")\n";

      // Mettre à jour les positions
      pos.x += vel.vx;
      pos.y += vel.vy;

      std::cout << "Entity " << entity_count - 1 << ": Position after update: ("
                << pos.x << ", " << pos.y << ")\n";
    }

    if (entity_count == 1) {
      std::cout << "[SUCCESS] Correct number of entities in the zipper.\n";
    } else {
      std::cout << "[ERROR] Incorrect number of entities in the zipper.\n";
    }
  } catch (const std::runtime_error& e) {
    std::cerr << "[ERROR] Exception caught in zipper: " << e.what() << "\n";
  }
}

int main() {
  std::cout << "=== Running ECS Tests ===\n";

  std::cout << "\n--- Test: Basic Operations ---\n";
  test_basic_operations();

  std::cout << "\n--- Test: Component Removal ---\n";
  test_component_removal();

  std::cout << "\n--- Test: Entity Reuse ---\n";
  test_entity_reuse();

  std::cout << "\n--- Test: System Execution ---\n";
  test_system_execution();

  std::cout << "\n--- Test: Get Entities With Components ---\n";
  test_get_entities_with_components();

  std::cout << "\n--- Test: Filtered Zipper ---\n";
  test_filtered_zipper();

  std::cout << "\n=== All Tests Completed ===\n";
  return 0;
}
