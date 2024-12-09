#ifndef COMPONENTS_HPP_
#define COMPONENTS_HPP_
#include <iostream>

struct Position {
    float x{0.0f};
    float y{0.0f};
};

struct Velocity {
    float vx{0.0f};
    float vy{0.0f};
};

struct Health {
    int value;
};

struct Collider {
    float width;
    float height;
    bool is_active;

    Collider(const float width, const float height, const bool is_active = true)
        : width(width), height(height), is_active(is_active) {}
};

struct Player {
    uint8_t id;
};

struct Actions {
    uint16_t current_actions;
};

struct DirtyFlag {
    bool is_dirty{true};
};

struct Projectile {
    uint8_t owner_id;
    uint8_t projectile_id;
};

struct LastShotTime {
    std::chrono::milliseconds last_shot_time{0};
};

#include <deque>
#include <chrono>

struct PositionHistory {
    struct Snapshot {
        Position position;
        std::chrono::milliseconds timestamp;
    };

    static constexpr size_t kMaxHistorySize = 100;
    std::deque<Snapshot> snapshots;

    void AddSnapshot(const Position& pos,
                     const std::chrono::milliseconds timestamp) {
        snapshots.emplace_back(Snapshot{pos, timestamp});
        while (snapshots.size() > kMaxHistorySize) {
            snapshots.pop_front();
        }
    }

    /*
    std::optional<Position> GetInterpolatedPosition(
         const std::chrono::milliseconds render_time) const {
        if (snapshots.size() < 2) {
            std::cerr << "[Interpolation][DEBUG] Not enough snapshots for interpolation\n";
            return {};
        }

        const auto it1 = std::ranges::find_if(snapshots,
                                [&](const Snapshot& snapshot) {
                                    return snapshot.timestamp >= render_time;
                                });

        if (it1 == snapshots.begin() || it1 == snapshots.end()) {
            std::cerr << "[Interpolation][DEBUG] Render time " << render_time.count()
                      << " ms is out of snapshot range\n";
            return {};
        }

        const auto it0 = std::prev(it1);

        const auto& [position0, timestamp0] = *it0;
        const auto& [position1, timestamp1] = *it1;

        const float alpha = static_cast<float>(
            (render_time - timestamp0).count()) /
            static_cast<float>((timestamp1 - timestamp0).count());

        std::cout << "[Interpolation][DEBUG] Snapshot0: Position(" << position0.x << ", " << position0.y
                  << "), Timestamp: " << timestamp0.count() << " ms\n";
        std::cout << "[Interpolation][DEBUG] Snapshot1: Position(" << position1.x << ", " << position1.y
                  << "), Timestamp: " << timestamp1.count() << " ms\n";
        std::cout << "[Interpolation][DEBUG] Render time: " << render_time.count()
                  << " ms, Alpha: " << alpha << '\n';

        return Position{
            position0.x + alpha * (position1.x - position0.x),
            position0.y + alpha * (position1.y - position0.y)
        };
    }
    */

    std::optional<Position> GetInterpolatedPosition(
    const Position& current_position,
    const std::chrono::milliseconds render_time) {
    if (snapshots.size() < 3) {
        std::cerr << "[Interpolation][DEBUG] Not enough snapshots for quadratic interpolation\n";
        return current_position;
    }

    // Trouver les snapshots correspondant à l'intervalle de temps
    const auto it_next = std::ranges::find_if(snapshots, [&](const Snapshot& snapshot) {
        return snapshot.timestamp >= render_time;
    });

    if (it_next == snapshots.end() || it_next == snapshots.begin()) {
        std::cerr << "[Interpolation][DEBUG] Render time out of range\n";
        return current_position;
    }

    const auto it_prev = std::prev(it_next);
    const auto it_next_next = std::next(it_next);

    if (it_next_next == snapshots.end()) {
        std::cerr << "[Interpolation][DEBUG] Not enough snapshots for quadratic interpolation\n";
        return current_position;
    }

    // Points pour l'interpolation
    const auto& [position0, timestamp0] = *it_prev;
    const auto& [position1, timestamp1] = *it_next;
    const auto& [position2, timestamp2] = *it_next_next;

    // Calcul de l'alpha pour le temps entre P0 et P1
    const float alpha = static_cast<float>((render_time - timestamp0).count()) /
                        static_cast<float>((timestamp1 - timestamp0).count());

    // Calcul de l'interpolation quadratique
    const Position interpolated_position{
        (1 - alpha) * (1 - alpha) * position0.x +
        2 * (1 - alpha) * alpha * position1.x +
        alpha * alpha * position2.x,

        (1 - alpha) * (1 - alpha) * position0.y +
        2 * (1 - alpha) * alpha * position1.y +
        alpha * alpha * position2.y
    };

    std::cout << "[Interpolation][DEBUG] P0: (" << position0.x << ", " << position0.y
              << "), P1: (" << position1.x << ", " << position1.y
              << "), P2: (" << position2.x << ", " << position2.y
              << "), Alpha: " << alpha << '\n';

    return interpolated_position;
}


};

#endif  // COMPONENTS_HPP_
