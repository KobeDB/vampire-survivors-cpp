#ifndef LEVEL_H
#define LEVEL_H

#include "raylib.h"
#include "rlgl.h"

#include "pool.h"
#include "weapons.h"
#include "constants.h"
#include "basic.h"
#include "entities.h"

#define MAX_ENEMIES 2000
#define MAX_DAMAGE_ZONES 500
#define MAX_WEAPONS 10
#define MAX_DAMAGE_INDICATORS 10000
#define MAX_XP_DROPS 10000
#define MAX_COUNTDOWNS 1000

struct Level {
    Camera2D                camera {};
    Player                  player {};
    Pool<Enemy>             enemies {MAX_ENEMIES};
    Pool<Damage_Zone>       damage_zones {MAX_DAMAGE_ZONES};
    Raw_Pool                weapons {MAX_WEAPONS, sizeof(Weapon_Union)};
    // Pool<Damage_Indicator>  damage_indicators{MAX_DAMAGE_INDICATORS;
    // Pool<XP_Drop>           xp_drops{MAX_XP_DROPS};
    // Wave                    wave{};
    // Pool<Countdown>         countdowns{MAX_COUNTDOWNS};

    void init(Vec2 screen_dim) {
        player.init();

        camera.target = {player.pos.x(), player.pos.y()};
        camera.offset = {screen_dim.x() / 2, screen_dim.y() / 2};
        camera.zoom = 1;

        for (int i = 0; i < 1; ++i) {
            enemies.add(make_enemy(Bat, player.pos + random_unit_vec<2>() * 1000));
        }

        weapons.add(Whip{damage_zones});
        weapons.add(Bibles{3, damage_zones});
        weapons.add(Magic_Wand{damage_zones});
        weapons.add(Cross{});
        weapons.add(Fire_Wand{});
    }

    void update_camera() {
        camera.target = {player.pos.x(), player.pos.y()};
    }

    void tick() {
        player.tick();

        update_camera();

        // Tick enemies
        for (int ei = 0; ei < enemies.capacity(); ++ei) {
            auto enemy = enemies.get(ei);
            if (!enemy) { continue; }
            enemy->tick(player);
        }

        // Tick weapons
        for (int i = 0; i < weapons.capacity(); ++i) {
            Weapon *weapon = (Weapon*)weapons.get(i);
            if (!weapon) { continue; }
            weapon->tick(player, damage_zones, enemies);
        }
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

            for(int ei = 0; ei < enemies.capacity(); ++ei) {
                auto enemy = enemies.get(ei);
                if (!enemy) { continue; }
                enemy->draw();
            }

            // Draw weapons
            for( int i = 0; i < weapons.capacity(); ++i) {
                Weapon *weapon = (Weapon*)weapons.get(i);
                if (!weapon) { continue; }
                weapon->draw();
            }

            // Draw damage zones (for debug purposes)
            for( int di = 0; di < damage_zones.capacity(); ++di) {
                auto dz = damage_zones.get(di);
                if (!dz) { continue; }
                dz->draw();
            }

        EndMode2D();
    }
};

#endif