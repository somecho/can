#pragma once

#include <SDL2/SDL.h>
#include <functional>
#include <memory>

namespace Can {

namespace SDLptr {

using Window = std::unique_ptr<SDL_Window, std::function<void(SDL_Window*)>>;
using Renderer =
    std::unique_ptr<SDL_Renderer, std::function<void(SDL_Renderer*)>>;

}  // namespace SDLptr

}  // namespace Can
