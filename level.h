#ifndef LEVEL_H
#define LEVEL_H

#include "raylib.h"
#include "rlgl.h"

#include "pool.h"
//#include "weapons.h"
#include "constants.h"
#include "basic.h"
#include "entities.h"

#define MAX_ENEMIES 10000
#define MAX_DAMAGE_ZONES 500
#define MAX_WEAPONS 10
#define MAX_DAMAGE_INDICATORS 10000
#define MAX_XP_DROPS 10000
#define MAX_COUNTDOWNS 1000

struct Level {
    Camera2D                camera {};
    Player                  player {};
    // Pool<Entity>            enemies{MAX_ENEMIES};
    // Pool<Damage_Zones>      damage_zones{MAX_DAMAGE_ZONES};
    // Weapon_Pool             weapons{MAX_WEAPONS};
    // Pool<Damage_Indicator>  damage_indicators{MAX_DAMAGE_INDICATORS;
    // Pool<XP_Drop>           xp_drops{MAX_XP_DROPS};
    // Wave                    wave{};
    // Pool<Countdown>         countdowns{MAX_COUNTDOWNS};

    void init(Vec2 screen_dim) {
        player.init();

        camera.target = {player.pos.x, player.pos.y};
        camera.offset = {screen_dim.x / 2, screen_dim.y / 2};
        camera.zoom = 1;
    }

    void update_camera() {
        camera.target = {player.pos.x, player.pos.y};
    }

    void tick() {
        player.do_movement();

        update_camera();
    }

    void draw() {
        BeginMode2D(camera);

            // Draw grid
            rlPushMatrix();
            rlTranslatef(0, 25*50, 0);
            rlRotatef(90, 1, 0, 0);
            DrawGrid(100, 50);
            rlPopMatrix();

            // Draw entities
            player.draw();

        EndMode2D();
    }
};

#endif