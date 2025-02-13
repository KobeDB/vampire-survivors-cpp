#ifndef LEVEL_H
#define LEVEL_H

#include "raylib.h"
#include "rlgl.h"

#include "pool.h"
#include "weapons.h"
#include "constants.h"
#include "basic.h"
#include "entities.h"
#include "quad_tree.h"

#define MAX_ENEMIES 3000
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
    Quad_Tree<Enemy*> enemy_quad_tree {{0,0}, {3000,3000}, 7};

    void init(Vec2 screen_dim) {
        player.init();

        camera.target = {player.pos.x(), player.pos.y()};
        camera.offset = {screen_dim.x() / 2, screen_dim.y() / 2};
        camera.zoom = 0.8f;

        for (int i = 0; i < MAX_ENEMIES; ++i) {
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

        // (Re)build enemy quad tree
        // TODO: Center enemy quad tree around player and not around world origin
        enemy_quad_tree.clear(); // first clear the tree from the last frame
        for (int ei = 0; ei < enemies.capacity(); ++ei) {
            auto enemy = enemies.get(ei);
            if (!enemy) { continue; }
            enemy_quad_tree.add_entity_quad(enemy, enemy->pos, enemy->dim);
        }

        separate_enemies();
    }

    void separate_enemies() {
        for (int i = 0; i < enemies.capacity(); ++i) {
            Enemy *e0 = enemies.get(i);
            if (!e0) { continue; }

            //if (!is_pos_in_view(e0->pos)) { continue; } // only handle collisions for enemies in view

            Vec2 influence_zone_dim = e0->dim * 2.0f;
            auto search_result = enemy_quad_tree.search(e0->pos, influence_zone_dim);

            for (int l = 0; l < 4; ++l) {
                Quad_Tree_Leaf<Enemy*> *leaf = search_result.leaves[l];
                if (!leaf) {continue;}
                for (int j = 0; j < leaf->entity_count; ++j) {
                    Enemy *e1 = leaf->entities[j];
                    if (e0 == e1) { continue; }

                    float d = length(e1->pos - e0->pos);
                    float thresh = influence_zone_dim.x()/2.0f;
                    if (d < thresh && d > 0) {
                        float repulsion = (1-d/thresh) * 10000.0f;
                        Vec2 e1_to_e0 = normalize(e0->pos - e1->pos);
                        e0->velocity += repulsion * e1_to_e0 * TICK_TIME;
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

    void draw_enemy_quad_tree_bounds() {
        float w = enemy_quad_tree.root_dimensions.x();
        float h = enemy_quad_tree.root_dimensions.y();
        float x = enemy_quad_tree.root_origin.x() - w/2.0f;
        float y = enemy_quad_tree.root_origin.y() - h/2.0f;
        DrawRectangleLines(x, y, w, h, RED);
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

            draw_enemy_quad_tree_bounds();


        EndMode2D();
    }
};

#endif