#ifndef SDL_TEXTURE_HPP_
#define SDL_TEXTURE_HPP_

#include <iostream>
#include <stdexcept>

// SDL2
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

namespace sdl {
class texture {
 public:
    constexpr texture(SDL_Renderer* renderer, TTF_Font* font = nullptr) noexcept
      : m_renderer(renderer), m_font(font) { }

    // Deallocates memory
    inline ~texture() noexcept { dealloc(); }

    // Creates image from font string
    void loadFromText(const char* text, SDL_Color color) noexcept(false) {
        if (m_font == nullptr) {
            return;
        }
        // Get rid of preexisting texture
        dealloc();

        // Render text surface
        SDL_Surface* text_surface = TTF_RenderText_Solid(m_font, text, color);
        if (text_surface == nullptr) {
            throw std::runtime_error(SDL_GetError());
        }

        // Create texture from surface pixels
        m_texture = SDL_CreateTextureFromSurface(m_renderer, text_surface);
        if (m_texture == nullptr) {
            std::cerr << "Error creating texture\n";
            throw std::runtime_error(SDL_GetError());
        }

        // Get image dimensions
        m_width  = text_surface->w;
        m_height = text_surface->h;

        // Get rid of old surface
        SDL_FreeSurface(text_surface);
    }

    // Deallocates texture
    void dealloc() noexcept {
        if (m_texture != nullptr) {
            SDL_DestroyTexture(m_texture);
            m_texture = nullptr;
            m_width   = 0;
            m_height  = 0;
        }
    }

    // Set blending
    void setBlendMode(SDL_BlendMode blending);

    // Set alpha modulation
    void setAlpha(Uint8 alpha);

    inline void create(Uint32 format, int access, int width, int height) {
        m_texture = SDL_CreateTexture(m_renderer, format, access, width, height);
        if (m_texture == nullptr) {
            std::cerr << "Error creating texture\n";
            throw std::runtime_error(SDL_GetError());
        }

        m_width  = width;
        m_height = height;
    }

    inline void render(const SDL_Rect* srcrect, const SDL_Rect* dstrect) {
        SDL_RenderCopy(m_renderer, m_texture, srcrect, dstrect);
    }

    // Get raw pointer
    [[nodiscard]] auto get() noexcept -> SDL_Texture* { return m_texture; }

    // Gets image dimensions
    [[nodiscard]] auto getWidth() const noexcept -> std::int32_t { return m_width; }
    [[nodiscard]] auto getHeight() const noexcept -> std::int32_t { return m_height; }

 private:
    // The actual renderer
    SDL_Renderer* m_renderer = nullptr;

    // The actual hardware texture
    SDL_Texture* m_texture = nullptr;

    // The actual font
    TTF_Font* m_font = nullptr;

    // Image dimensions
    std::int32_t m_width{};
    std::int32_t m_height{};
};
}  // namespace sdl

#endif  // SDL_TEXTURE_HPP_