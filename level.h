#ifndef LEVEL_H
#define LEVEL_H

#include "raylib.h"
#include "rlgl.h"

#include "pool.h"
#include "weapons.h"
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
        camera.zoom = 1.0f;

        for (int i = 0; i < 2000; ++i) {
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

        // Tick weapons
        for (int i = 0; i < weapons.capacity(); ++i) {
            Weapon *weapon = (Weapon*)weapons.get(i);
            if (!weapon) { continue; }
            weapon->tick(player, damage_zones, enemies);
        }

        // Tick enemies
        for (int ei = 0; ei < enemies.capacity(); ++ei) {
            auto enemy = enemies.get(ei);
            if (!enemy) { continue; }
            enemy->tick(player);
        }

        separate_enemies();
    }

    void separate_enemies() {
        const int max_iterations = 3;
        int iteration = 0;
        bool collisions_remaining = true;
        while (collisions_remaining && iteration < max_iterations) {
            collisions_remaining = false;
            for (int i = 0; i < enemies.capacity(); ++i) {
                Enemy *e0 = enemies.get(i);
                if (!e0) { continue; }
                
                if (!is_pos_in_view(e0->pos)) { continue; } // only handle collisions for enemies in view
                
                for (int j = i + 1; j < enemies.capacity(); ++j) {
                    Enemy *e1 = enemies.get(j);
                    if (!e1) { continue; }
                    
                    float r0 = e0->dim.x()/2.0f;
                    float r1 = e1->dim.x()/2.0f;
                    float d = length(e1->pos - e0->pos);
                    float overlap = r0+r1 - d;
                    if (overlap > 0) {
                        if (d == 0) { d = 0.01f; }
                        Vec2 e0_to_e1 = normalize(e1->pos - e0->pos);
                        Vec2 e1_to_e0 = normalize(e0->pos - e1->pos);
                        e0->pos += e1_to_e0 * overlap/2.0f;
                        e1->pos += e0_to_e1 * overlap/2.0f;
                    }
                }
            }
        }
    }

    bool is_pos_in_view(Vec2 pos) const {
        Vector2 top_left = GetScreenToWorld2D({0,0}, camera);
        Vector2 bottom_right = GetScreenToWorld2D({(float)GetScreenWidth(),(float)GetScreenHeight()}, camera);
        return pos.x() > top_left.x && pos.x() < bottom_right.x && pos.y() > top_left.y && pos.y() < bottom_right.y;
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