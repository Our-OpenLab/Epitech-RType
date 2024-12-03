#include <SDL2/SDL.h>
#include <ecs/registry.hpp>
#include <ecs/zipper.hpp>
#include <iostream>
#include <chrono>

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
  auto& positions = reg.get_components<position>();
  auto& velocities = reg.get_components<velocity>();

  for (auto&& [pos, vel] : ecs::Zipper(positions, velocities)) {
    if (pos && vel) {
      pos->x += vel->vx;
      pos->y += vel->vy;
    }
  }
}

void draw_system(ecs::Registry<position, velocity, drawable>& reg, SDL_Renderer* renderer) {
  auto& drawables = reg.get_components<drawable>();
  auto& positions = reg.get_components<position>();

  for (auto&& [draw, pos] : ecs::Zipper(drawables, positions)) {
    if (draw && pos) {
      draw->rect.x = static_cast<int>(pos->x);
      draw->rect.y = static_cast<int>(pos->y);
      SDL_RenderCopy(renderer, draw->texture, nullptr, &draw->rect);
    }
  }
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("ECS with SDL2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    ecs::Registry<position, velocity, drawable> reg;

    reg.register_component<position>();
    reg.register_component<velocity>();
    reg.register_component<drawable>();

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 50, 50);
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
                running = false;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        reg.run_systems();

        SDL_RenderPresent(renderer);

        auto frame_time = SDL_GetTicks() - frame_start;

        if (frame_delay > frame_time) {
            SDL_Delay(frame_delay - frame_time);
        }
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
