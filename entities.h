#ifndef ENTITIES_H
#define ENTITIES_H

#include "raylib.h"
#include "rlgl.h"

#include "basic.h"

#include "resources.h"

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
        if (flip_x) { frame_dim.x *= -1; }
        if (flip_x_by_default) { frame_dim.x *= -1; }

        auto frame_rec = Rectangle{ frame_pos.x, frame_pos.y, frame_dim.x, frame_dim.y };

        auto dest_rec_dim = Vec2{ 75 * scaling.x, 75 * scaling.y };
        auto dest_rec_pos = pos - dest_rec_dim/2.0f;

        auto dest_rec = Rectangle{ dest_rec_pos.x, dest_rec_pos.y, dest_rec_dim.x, dest_rec_dim.y };

        DrawTexturePro(texture, frame_rec, dest_rec, {}, 0, WHITE);
    }
};

#define PLAYER_START_REQ_XP 5
struct Player {
    Vec2 pos {};
    Vec2 dim {};
    float move_speed {};
    Vec2 facing_dir {};

    int cur_level {};
    int target_level {};

    int req_xp {};
    int cur_xp {};

    Animation animation {};

    void init() {
        pos = {10,10};
        dim = {75,75};
        move_speed = 170;
        facing_dir = {1,0};
        cur_level = 0;
        target_level = cur_level;
        req_xp = PLAYER_START_REQ_XP;
        cur_xp = 0;

        animation.init(10, 6, get_texture("scarfy"), false);
    }

    void do_movement() {
        Vec2 move_dir{};
        if (IsKeyDown(KEY_D)) {
            move_dir.x += 1;
            facing_dir = {1,0};
            printf("D pressed\n");
        }
        if (IsKeyDown(KEY_A)) {
            move_dir.x -= 1;
            facing_dir = {-1,0};
        }
        if (IsKeyDown(KEY_S)) {
            move_dir.y += 1;
        }
        if (IsKeyDown(KEY_W)) {
            move_dir.y -= 1;
        }
        if (length(move_dir) != 0) {
            move_dir = normalize(move_dir);
            animation.tick();
        }
        pos += move_speed * move_dir * TICK_TIME;
        printf("Pos: %f, %f\n", pos.x, pos.y);
    }

    void draw() {
        // auto corner = pos - dim/2;
        // DrawRectangle(corner.x, corner.y, dim.x, dim.y, MAGENTA);
        animation.draw(pos, facing_dir.x <= 0);
    }
};

struct Entity {
    Vec2 pos;
    Vec2 dim;
    Vec2 velocity;
    float max_move_speed;
    float health;
    Color color;
};


#endif