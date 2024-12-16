#ifndef COMPONENTS_HPP_
#define COMPONENTS_HPP_
#include <iostream>

struct Circle {
    float radius;
};

struct Rectangle {
    float width, height;
};

using Shape = std::variant<Circle, Rectangle>;

struct Health {
    int value;
};

struct ServerPlayer {
    uint8_t id;
    Shape shape;
    uint16_t score{0};
    uint8_t health{100};
};

struct ClientPlayer {
    uint8_t id;
    uint16_t score{0};
    uint8_t health{100};
};

struct Enemy {
    uint8_t id;
    Shape shape;
};

struct Projectile {
    uint8_t owner_id;
    uint8_t projectile_id;
    Shape shape;
    int damage;
};

struct Actions {
    uint16_t current_actions;
};

struct Position {
    float x{0.0f};
    float y{0.0f};
};

struct Velocity {
    float vx{0.0f};
    float vy{0.0f};
};

struct DirtyFlag {
    bool is_dirty{true};
};

struct LastShotTime {
    std::chrono::milliseconds last_shot_time{0};
};

struct AIState {
    enum State {
        Idle,
        Patrol,
        Pursue,
        Attack,
        Flee
    } state{Idle};
};

struct PatrolPath {
    std::vector<Position> waypoints;  // Liste des points de patrouille
    size_t current_index = 0;         // Index du waypoint actuel
    bool loop = true;                 // Revenir au début une fois la fin atteinte
};

struct Target {
    uint8_t target_id;
    bool has_target{false};
};

struct Flocking {
    float cohesion_weight{1.0f};
    float separation_weight{1.5f};
    float alignment_weight{1.0f};
    float neighbor_radius{50.0f};  // Distance maximale pour considérer un voisin
};

struct Aggro {
    float range{100.0f};  // Distance d'agression
    bool is_aggroed{false};  // Indique si l'ennemi est en état d'agression
};

inline bool is_collision(const Circle& c1, const float x1, const float y1,
                         const Circle& c2, const float x2, const float y2) {
  const float dx = x1 - x2;
  const float dy = y1 - y2;
  const float distance_squared = dx * dx + dy * dy;
  const float radius_sum = c1.radius + c2.radius;
  return distance_squared <= radius_sum * radius_sum;
}

inline bool is_collision(const Rectangle& r1, const float x1, const float y1,
                         const Rectangle& r2, const float x2, const float y2) {
  return !(x1 > x2 + r2.width || x2 > x1 + r1.width || y1 > y2 + r2.height ||
           y2 > y1 + r1.height);
}

inline bool is_collision(const Circle& circle, const float cx, const float cy,
                         const Rectangle& rect, const float rx,
                         const float ry) {
  const float closest_x = std::max(rx, std::min(cx, rx + rect.width));
  const float closest_y = std::max(ry, std::min(cy, ry + rect.height));
  const float dx = cx - closest_x;
  const float dy = cy - closest_y;
  return (dx * dx + dy * dy) <= circle.radius * circle.radius;
}

inline bool is_collision(const Rectangle& rect, const float rx, const float ry, const Circle& circle, const float cx, const float cy) {
    return is_collision(circle, cx, cy, rect, rx, ry);
}


#endif  // COMPONENTS_HPP_
