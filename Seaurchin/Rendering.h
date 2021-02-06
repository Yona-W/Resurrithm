#pragma once

#include <cstdint>
#include <SDL2/SDL.h>
#include <SDL_gpu.h>

namespace Rendering {

    typedef struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    } COLOR;

    typedef struct {
        float x;
        float y;
        float z;
    } VECTOR;

    typedef struct {
        VECTOR position;
        VECTOR normal;
        COLOR diffuse;
        COLOR specular;
        float u;
        float v;
    } VERTEX3D;

    typedef struct {
        VECTOR position;
        float rhw; // look this up it's matrix magic
        COLOR diffuse;
        float u;
        float v;
    } VERTEX2D;

    inline VECTOR VGet(float x, float y, float z) {
        return VECTOR{x, y, z};
    }

    inline COLOR GetColorU8(uint8_t r, uint8_t g, uint8_t b, uint8_t a){
        return COLOR{r, g, b, a};
    }

    void Render3DPolygon(const GPU_Image *texture, const GPU_Target *target, const VERTEX3D verts[], const uint16_t vertCount, const uint16_t indices[], const uint triCount);
    void Render2DPolygon(const GPU_Image *texture, const GPU_Target *target, const VERTEX2D verts[], const uint16_t vertCount, const uint16_t indices[], const uint triCount);
    void Set3DCamera();
    void Clear3DCamera();
    VECTOR WorldToScreen(VECTOR worldspace);

    extern GPU_Target *gpu;
}