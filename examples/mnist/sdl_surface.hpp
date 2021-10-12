#ifndef SDL_SURFACE_HPP_
#define SDL_SURFACE_HPP_

#include <iostream>
#include <stdexcept>

// SDL2
#include <SDL2/SDL.h>

namespace sdl {
class surface {
 public:
    constexpr surface(SDL_Surface* ptr) noexcept(false) : m_surface(ptr) {
        if (!m_surface) {
            throw std::runtime_error("Failed to allocate a surface");
        }
    }

    inline surface(int width, int height, int depth, std::uint32_t format) noexcept : surface(SDL_CreateRGBSurfaceWithFormat(0, width, height, depth, format)) { }

    inline surface(void* pixels, int width, int height, int depth, int pitch, std::uint32_t format) : surface(SDL_CreateRGBSurfaceWithFormatFrom(pixels, width, height, depth, pitch, format)) { }

    // Deallocates memory
    inline ~surface() noexcept { dealloc(); }

    // Deallocates texture
    void dealloc() noexcept {
        if (m_surface != nullptr) {
            SDL_FreeSurface(m_surface);
            m_surface = nullptr;
        }
    }

    /**
     *  You should call this unless you know exactly how SDL
     *  blitting works internally and how to use the other blit functions.
     *
     *  @upstream SDL_BlitSurface
     */
    void blit(SDL_Surface* src) noexcept(false) {
        const int& return_code = SDL_BlitSurface(src, nullptr, m_surface, nullptr);
        if (return_code != 0) {
            throw std::runtime_error("Failed to perform a blit of a surface");
        }
    }
    void blit(SDL_Surface* src, SDL_Rect* srcrect, SDL_Rect* dstrect) noexcept(false) {
        const int& return_code = SDL_BlitSurface(src, srcrect, m_surface, dstrect);
        if (return_code != 0) {
            throw std::runtime_error("Failed to perform a blit of a surface");
        }
    }

    /**
     * This is the public scaled blit function, and it performs
     * rectangle validation and clipping.
     *
     * @upstream SDL_BlitScaled
     */
    void blit_scaled(SDL_Surface* src) noexcept(false) {
        const int& return_code = SDL_BlitScaled(src, nullptr, m_surface, nullptr);
        if (return_code == -1) {
            throw std::runtime_error("Failed to perform scaled blit of a surface");
        }
    }
    void blit_scaled(SDL_Surface* src, SDL_Rect* srcrect, SDL_Rect* dstrect) noexcept(false) {
        const int& return_code = SDL_BlitScaled(src, srcrect, m_surface, dstrect);
        if (return_code == -1) {
            throw std::runtime_error("Failed to perform scaled blit of a surface");
        }
    }

    // Get raw pointer
    [[nodiscard]] auto get() noexcept -> SDL_Surface* { return m_surface; }

    // Gets image dimensions
    [[nodiscard]] auto width() const noexcept -> std::int32_t { return m_surface->w; }
    [[nodiscard]] auto height() const noexcept -> std::int32_t { return m_surface->h; }

    [[nodiscard]] auto pixels() noexcept -> void* { return m_surface->pixels; }

    [[nodiscard]] auto format() const -> SDL_PixelFormat* {
        return m_surface->format;
    }

 private:
    // The actual surface
    SDL_Surface* m_surface = nullptr;
};
}  // namespace sdl

#endif  // SDL_SURFACE_HPP_