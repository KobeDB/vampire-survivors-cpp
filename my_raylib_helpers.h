#ifndef MY_RAYLIB_HELPERS_H
#define MY_RAYLIB_HELPERS_H

#include "raylib.h"
#include "basic.h"

inline Color to_rl_color(const Vec3 &color) {
    return {
        (unsigned char)(color[0] * 255.0f),
        (unsigned char)(color[1] * 255.0f),
        (unsigned char)(color[2] * 255.0f),
        255 // Default alpha to fully opaque
    };
}

inline Color to_rl_color(const Vec4 &color) {
    return {
        (unsigned char)(color[0] * 255.0f),
        (unsigned char)(color[1] * 255.0f),
        (unsigned char)(color[2] * 255.0f),
        (unsigned char)(color[3] * 255.0f)
    };
}

#endif