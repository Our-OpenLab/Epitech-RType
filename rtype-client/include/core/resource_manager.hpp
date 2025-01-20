#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <unordered_map>
#include <iostream>

/**
 * @brief A global resource manager for SDL Textures and TTF Fonts.
 *
 * Scenes or other subsystems can Load/Unload assets by ID. Once loaded,
 * you can retrieve them via GetTexture/GetFont.
 *
 * If you want a reference-counted approach, you'd store usage info
 * and unload automatically when usage reaches zero. But here we keep it
 * simple: Scenes must explicitly Unload the assets they no longer need.
 */
class GlobalResourceManager
{
public:
    // Singleton-style access.
    static GlobalResourceManager& Instance()
    {
        static GlobalResourceManager instance;
        return instance;
    }

    /**
     * @brief Load an SDL_Texture from a file and store it under a given ID.
     * @param id        A unique string identifier for the texture.
     * @param filePath  The path to the image file.
     * @param renderer  An SDL_Renderer used to create the texture.
     * @return true on success, false on failure.
     */
    bool LoadTexture(const std::string& id,
                     const std::string& filePath,
                     SDL_Renderer* renderer)
    {
        if (textures_.contains(id)) {
            return true;
        }

        SDL_Surface* surface = SDL_LoadBMP(filePath.c_str());
        if (!surface) {
            std::cerr << "[ResourceManager] Failed to load BMP: "
                      << SDL_GetError() << std::endl;
            return false;
        }

        SDL_Texture* newTexture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        if (!newTexture) {
            std::cerr << "[ResourceManager] Failed to create texture: "
                      << SDL_GetError() << std::endl;
            return false;
        }

        textures_[id] = newTexture;
        return true;
    }

    /**
     * @brief Unload a texture by ID.
     * @param id  The texture's ID.
     */
    void UnloadTexture(const std::string& id)
    {
      if (const auto it = textures_.find(id); it != textures_.end()) {
            SDL_DestroyTexture(it->second);
            textures_.erase(it);
        }
    }

    /**
     * @brief Retrieve an SDL_Texture by ID (read-only).
     * @param id  The texture's ID.
     * @return SDL_Texture* or nullptr if not found.
     */
    SDL_Texture* GetTexture(const std::string& id) const
    {
      if (const auto it = textures_.find(id); it != textures_.end()) {
            return it->second;
        }
        return nullptr;
    }

    /**
     * @brief Load a TTF font by ID (with a given size).
     * @param id        A unique string identifier for the font.
     * @param filePath  Path to the .ttf file.
     * @param fontSize  The size to load the font at.
     * @return true on success, false on failure.
     */
    bool LoadFont(const std::string& id,
                  const std::string& filePath,
                  const int fontSize)
    {
      //  if (fonts_.contains(id)) {
      //      return true;
      //  }

        TTF_Font* newFont = TTF_OpenFont(filePath.c_str(), fontSize);
        if (!newFont) {
            std::cerr << "[ResourceManager] Failed to load font: "
                      << TTF_GetError() << std::endl;
            return false;
        }

        fonts_[id] = newFont;
        return true;
    }

    /**
     * @brief Unload a TTF font by ID.
     * @param id The font's ID.
     */
    void UnloadFont(const std::string& id)
    {
      if (const auto it = fonts_.find(id); it != fonts_.end()) {
            TTF_CloseFont(it->second);
            fonts_.erase(it);
        }
    }

    /**
     * @brief Retrieve a TTF_Font by ID (read-only).
     * @param id  The font's ID.
     * @return TTF_Font* or nullptr if not found.
     */
    TTF_Font* GetFont(const std::string& id) const
    {
      if (const auto it = fonts_.find(id); it != fonts_.end()) {
            return it->second;
        }
        return nullptr;
    }

private:
    GlobalResourceManager()  = default;  // private for singleton
    ~GlobalResourceManager() = default;  // cleanup on exit?

    // No copy
    GlobalResourceManager(const GlobalResourceManager&) = delete;
    GlobalResourceManager& operator=(const GlobalResourceManager&) = delete;

    std::unordered_map<std::string, SDL_Texture*> textures_;
    std::unordered_map<std::string, TTF_Font*>    fonts_;
};
