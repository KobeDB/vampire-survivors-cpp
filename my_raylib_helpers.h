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

inline void draw_texture(Texture2D tex, Vec2 pos, float scale, float rotation = 0.0f) {
    Rectangle src_rec = {0, 0, float(tex.width), float(tex.height)};
    Vec2 dest_rec_dim = Vec2{50,50} * scale;
    Rectangle dest_rec = {pos.x(), pos.y(), dest_rec_dim.x(), dest_rec_dim.y()};
    Vec2 origin = dest_rec_dim / 2.0f;
    DrawTexturePro(tex, src_rec, dest_rec, {origin.x(),origin.y()}, rotation, WHITE);
}

#endif