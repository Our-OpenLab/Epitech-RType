#include "core/renderer.hpp"

#include <SDL_ttf.h>

#include <iostream>
#include <stdexcept>

Renderer::Renderer(const std::string& title, int width, int height)
    : window(nullptr),
      sdlRenderer(nullptr),
      windowWidth(width),
      windowHeight(height)
{
    // 1. Initialize SDL (video subsystem, etc.)
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error(std::string("[Renderer] SDL_Init Error: ") + SDL_GetError());
    }

  if (TTF_Init() < 0) {
    SDL_Quit();
    throw std::runtime_error(std::string("[Renderer] TTF_Init Error: ") + TTF_GetError());
  }

    // 2. Create the window
    window = SDL_CreateWindow(title.c_str(),
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              windowWidth,
                              windowHeight,
                              SDL_WINDOW_SHOWN /* or e.g. SDL_WINDOW_OPENGL, etc. */);
    if (!window) {
        std::string err = "[Renderer] Failed to create window: ";
        err += SDL_GetError();
        throw std::runtime_error(err);
    }

    // 3. Create the renderer
    //    -1 : choose the first rendering driver that matches (usually hardware-accelerated)
    //     flags : use accelerated rendering, plus vsync if you want
    sdlRenderer = SDL_CreateRenderer(window, -1,
                                     SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!sdlRenderer) {
        std::string err = "[Renderer] Failed to create renderer: ";
        err += SDL_GetError();
        throw std::runtime_error(err);
    }

    // Optionally set an initial draw color if you like
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
}

Renderer::~Renderer()
{
    // Destroy the renderer if exists
    if (sdlRenderer) {
        SDL_DestroyRenderer(sdlRenderer);
        sdlRenderer = nullptr;
    }

    // Destroy the window if exists
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    // Quit SDL subsystems
    SDL_Quit();
}

void Renderer::Clear()
{
    // Clear the screen with the currently set draw color
    SDL_RenderClear(sdlRenderer);
}

void Renderer::Present()
{
    // Present the render buffer to the screen
    SDL_RenderPresent(sdlRenderer);
}

void Renderer::SetDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    SDL_SetRenderDrawColor(sdlRenderer, r, g, b, a);
}
