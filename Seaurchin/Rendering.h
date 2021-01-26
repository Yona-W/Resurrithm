#pragma once

#include <cstdint>
#include <SDL2/SDL.h>

namespace Rendering {
    typedef struct {
        double x;
        double y;
    } VERTEX2D;

    typedef struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    } COLOR;

    SDL_Window *window;
    SDL_Renderer *renderer;
}