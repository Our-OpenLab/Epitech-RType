#include <server/engine/game_state.hpp>

/*
void GameState::add_player(const uint8_t player_id, const float x,
                           const float y, const float speed) {
  const auto entity = registry_.spawn_entity();

  registry_.add_component<Player>(entity, {player_id});
  registry_.add_component<Position>(entity, {x, y});
  registry_.add_component<Velocity>(entity, {});
  //registry_.add_component<Health>(entity, {100});
  registry_.add_component<Actions>(entity, {});
  registry_.add_component<DirtyFlag>(entity, {});

  player_entities_[player_id] = entity;
}
*/

bool GameState::AddPlayer(const uint8_t player_id, const float x, const float y) {
  if (player_entities_.contains(player_id)) {
    std::cout << "[GameState][WARN] Player ID " << static_cast<int>(player_id)
              << " already exists. Skipping addition.\n";
    return false;
  }

  const auto entity = registry_.spawn_entity();

  registry_.emplace_component<Player>(entity, Player{player_id});
  registry_.emplace_component<Position>(entity, Position{x, y});
  registry_.emplace_component<Actions>(entity, Actions{0});
  registry_.emplace_component<DirtyFlag>(entity, DirtyFlag{true});
  registry_.emplace_component<LastShotTime>(entity, LastShotTime{});

  player_entities_[player_id] = entity;

  std::cout << "[GameState][INFO] Player " << static_cast<int>(player_id)
            << " added at position (" << x << ", " << y << ").\n";

  return true;
}


Player& GameState::get_player(uint8_t id) const {
  const auto it = player_entities_.find(id);
  if (it == player_entities_.end()) {
    throw std::runtime_error("Player with ID " + std::to_string(id) +
                             " not found.");
  }

  const auto entity = it->second;
  auto& players = registry_.get_components<Player>();

  if (!players[entity].has_value()) {
    throw std::runtime_error("Player entity does not have a Player component.");
  }

  return *players[entity];
}

Registry::entity_t GameState::get_entity_by_player_id(const uint8_t player_id) const {
  if (!player_entities_.contains(player_id)) {
    return static_cast<Registry::entity_t>(-1);
  }
  return player_entities_.at(player_id);
}

void GameState::remove_player(const uint8_t id) {
  if (!player_entities_.contains(id)) {
    throw std::runtime_error("Player ID not found");
  }

  const auto entity = player_entities_.at(id);

  registry_.kill_entity(entity);

  player_entities_.erase(id);

  std::cout << "[GameState] Player " << static_cast<int>(id) << " removed.\n";
}


std::vector<uint8_t> GameState::get_all_player_ids() const {
  //std::vector<uint8_t> ids;
  //auto& players = registry_.get_components<Player>();
  //for (const auto& player_opt : players) {
  //  if (player_opt.has_value()) {
  //    ids.push_back(player_opt->id);
  //  }
  //}
  //return ids;
  return {};
}
