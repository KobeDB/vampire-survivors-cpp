#ifndef ENTITIES_H
#define ENTITIES_H

#include "raylib.h"
#include "rlgl.h"

#include "basic.h"

#include "resources.h"

#include "animation.h"

#define PLAYER_START_REQ_XP 5
struct Player {
    Vec2 pos {};
    Vec2 dim {};
    float move_speed {};
    Vec2 velocity {};
    Vec2 facing_dir {};

    int cur_level {};
    int target_level {};

    int req_xp {};
    int cur_xp {};

    Animation animation {};

    void init() {
        pos = {10,10};
        dim = {75,75};
        move_speed = 250;
        facing_dir = {1,0};
        cur_level = 0;
        target_level = cur_level;
        req_xp = PLAYER_START_REQ_XP;
        cur_xp = 0;

        animation.init(10, 6, get_texture("scarfy"), false);
    }

    void tick() {
        Vec2 move_dir{};
        if (IsKeyDown(KEY_D)) {
            move_dir.x() += 1;
            facing_dir = {1,0};
        }
        if (IsKeyDown(KEY_A)) {
            move_dir.x() -= 1;
            facing_dir = {-1,0};
        }
        if (IsKeyDown(KEY_S)) {
            move_dir.y() += 1;
        }
        if (IsKeyDown(KEY_W)) {
            move_dir.y() -= 1;
        }
        if (length(move_dir) != 0) {
            move_dir = normalize(move_dir);
            animation.tick();
        }
        velocity = move_dir * move_speed;
        pos += velocity * TICK_TIME;
    }

    void draw() {
        Vec2 corner = pos - dim/2;
        DrawRectangleLines(corner.x(), corner.y(), dim.x(), dim.y(), MAGENTA);
        animation.draw(pos, facing_dir.x() <= 0);
    }
};

struct Enemy {
    Vec2 pos {};
    Vec2 dim {};
    Vec2 velocity {};
    Vec2 force {};
    float max_move_speed {};
    float health {};
    Color color {};
    Animation animation {};

    void tick(const Player &player) {
        Vec2 to_player = normalize(player.pos - pos);

        Vec2 to_player_force = to_player * 500.0f;
        force += to_player_force;

        // Apply resistance force to to_player_force based on how close the enemy is to its max_move_speed
        // If the enemy's velocity towards the player has reached max_move_speed it'll stay at that speed 
        float alpha = dot(velocity, to_player) / max_move_speed;
        force += alpha * -to_player_force;

        // Apply friction on perpendicular axis to to_player axis
        // This will prevent the enemies from permanently orbiting around the player
        Vec2 to_player_perp = {-to_player.y(), to_player.x()};
        Vec2 perp_velocity = to_player_perp * dot(velocity, to_player_perp);
        Vec2 friction = -perp_velocity;
        force += friction;

        velocity += force * TICK_TIME;
        
        pos += velocity * TICK_TIME;

        animation.tick();
    }

    void draw() const {
        Vec2 corner = pos - dim/2;
        DrawRectangleLines(corner.x(), corner.y(), dim.x(), dim.y(), RED);
        animation.draw(pos, velocity.x() < 0);
    }
};

enum Enemy_Type {
    Bat
};

inline Enemy make_enemy(Enemy_Type type, Vec2 pos) {
    Enemy result {};
    result.pos = pos;

    switch (type) {
        case Bat: {
            result.dim = {40,40};
            result.max_move_speed = 100;
            result.health = 100;
            result.color = MAROON;
            result.animation.init(5, 8, get_texture("bat"), true);
            result.animation.scaling = {2,2};
        } break;
        default: {} break;
    }

    return result;
}


#endif