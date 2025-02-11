#ifndef ANIMATION_H
#define ANIMATION_H

#include "raylib.h"

#include "basic.h"

struct Animation {
    int frame_elapsed_ticks {};
    int frame_time_ticks {};
    int frame {};
    int frame_count {};
    float frame_width {};
    Texture2D texture {};
    bool flip_x_by_default {};
    Vec2 scaling {1,1};

    void init(int p_frame_time_ticks, int p_frame_count, Texture2D p_texture, bool flip_x = false) {
        frame_time_ticks = p_frame_time_ticks;
        frame_count = p_frame_count;
        texture = p_texture;
        flip_x_by_default = flip_x;

        frame_width = float(texture.width) / float(frame_count);
    }

    void tick() {
        if (frame_count == 0) { return; }

        frame_elapsed_ticks += 1;
        if (frame_elapsed_ticks >= frame_time_ticks) {
            frame_elapsed_ticks = 0;
            frame = (frame + 1) % frame_count;
        }
    }

    void draw(Vec2 pos, bool flip_x) const {
        auto frame_pos = Vec2{ frame * frame_width, 0 };
        auto frame_dim = Vec2{ frame_width, float(texture.height) };
        if (flip_x) { frame_dim.x() *= -1; }
        if (flip_x_by_default) { frame_dim.x() *= -1; }

        auto frame_rec = Rectangle{ frame_pos.x(), frame_pos.y(), frame_dim.x(), frame_dim.y() };

        Vec2 dest_rec_dim = Vec2{ 75 * scaling.x(), 75 * scaling.y() };
        Vec2 dest_rec_pos = pos - dest_rec_dim/2.0f;

        auto dest_rec = Rectangle{ dest_rec_pos.x(), dest_rec_pos.y(), dest_rec_dim.x(), dest_rec_dim.y() };

        DrawTexturePro(texture, frame_rec, dest_rec, {}, 0, WHITE);
    }
};

#endif