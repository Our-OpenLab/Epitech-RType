#include <SDL2/SDL.h>
#include <new_ecs/registry.hpp>
#include <new_ecs/zipper.hpp>
#include <iostream>
#include <vector>

struct position {
    float x, y;
};

struct velocity {
    float vx, vy;
};

struct drawable {
    SDL_Texture* texture;
    SDL_Rect rect;
};

void position_system(ecs::Registry<position, velocity, drawable>& reg) {
    std::cout << "Running position_system...\n";

    auto zipper = reg.get_filtered_zipper<position, velocity>();
    size_t entity_count = 0;

    for (auto&& [pos, vel] : zipper) {
        std::cout << "Entity " << entity_count++ << ": Position before update: ("
                  << pos.x << ", " << pos.y << "), Velocity: ("
                  << vel.vx << ", " << vel.vy << ")\n";

        pos.x += vel.vx;
        pos.y += vel.vy;

        std::cout << "Entity " << entity_count - 1 << ": Position after update: ("
                  << pos.x << ", " << pos.y << ")\n";
    }

    if (entity_count == 0) {
        std::cout << "No entities updated in position_system.\n";
    }
}

void draw_system(ecs::Registry<position, velocity, drawable>& reg, SDL_Renderer* renderer) {
    std::cout << "Running draw_system...\n";

    auto zipper = reg.get_filtered_zipper<position, drawable>();
    size_t entity_count = 0;

    for (auto&& [pos, draw] : zipper) {
        std::cout << "Entity " << entity_count++
                  << ": Position: (" << pos.x << ", " << pos.y
                  << "), Rect before update: (" << draw.rect.x << ", "
                  << draw.rect.y << ", " << draw.rect.w << ", " << draw.rect.h << ")\n";

        draw.rect.x = static_cast<int>(pos.x);
        draw.rect.y = static_cast<int>(pos.y);

        if (SDL_RenderCopy(renderer, draw.texture, nullptr, &draw.rect) != 0) {
            std::cerr << "SDL_RenderCopy failed for entity " << entity_count - 1
                      << ": " << SDL_GetError() << "\n";
        } else {
            std::cout << "Entity " << entity_count - 1 << " rendered at: ("
                      << draw.rect.x << ", " << draw.rect.y << ")\n";
        }
    }

    if (entity_count == 0) {
        std::cout << "No entities rendered in draw_system.\n";
    }
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "ECS with SDL2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    ecs::Registry<position, velocity, drawable> reg;

    reg.register_component<position>();
    reg.register_component<velocity>();
    reg.register_component<drawable>();

    SDL_Texture* texture = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 50, 50);
    if (!texture) {
        std::cerr << "Failed to create texture: " << SDL_GetError() << "\n";
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    } else {
        std::cout << "Texture created successfully.\n";
    }

    SDL_SetRenderTarget(renderer, texture);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, nullptr);

    auto entity1 = reg.spawn_entity();
    reg.add_component(entity1, position{100.0f, 100.0f});
    reg.add_component(entity1, velocity{0.2f, 0.15f});
    reg.add_component(entity1, drawable{texture, {0, 0, 50, 50}});

    auto entity2 = reg.spawn_entity();
    reg.add_component(entity2, position{200.0f, 300.0f});
    reg.add_component(entity2, velocity{-0.1f, -0.1f});
    reg.add_component(entity2, drawable{texture, {0, 0, 50, 50}});

    reg.add_system(position_system);
    reg.add_system([&](ecs::Registry<position, velocity, drawable>& reg) {
        draw_system(reg, renderer);
    });

    const int FPS = 60;
    const int frame_delay = 1000 / FPS;

    bool running = true;
    SDL_Event event;

    while (running) {
        auto frame_start = SDL_GetTicks();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                std::cout << "Quit event detected. Exiting...\n";
                running = false;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        std::cout << "Running ECS systems...\n";
        reg.run_systems();

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect debug_rect = {10, 10, 100, 100};
        if (SDL_RenderDrawRect(renderer, &debug_rect) != 0) {
            std::cerr << "SDL_RenderDrawRect failed: " << SDL_GetError() << "\n";
        } else {
            std::cout << "Debug rectangle drawn.\n";
        }

        SDL_RenderPresent(renderer);

        auto frame_time = SDL_GetTicks() - frame_start;

        if (frame_delay > frame_time) {
            SDL_Delay(frame_delay - frame_time);
        }

        std::cout << "Frame completed. Frame time: " << frame_time << "ms\n";
        exit(0);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
