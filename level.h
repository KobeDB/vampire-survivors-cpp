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
#define MAX_DAMAGE_INDICATORS 100000
#define MAX_XP_DROPS 10000
#define MAX_COUNTDOWNS 1000

struct Damage_Indicator {
    Vec2 pos;
    int damage;
    bool critical_hit = false;
    int lifetime = 10;
};

struct Level {
    Camera2D                camera {};
    Player                  player {};
    Pool<Enemy>             enemies {MAX_ENEMIES};
    Pool<Damage_Zone>       damage_zones {MAX_DAMAGE_ZONES};
    Raw_Pool                weapons {MAX_WEAPONS, sizeof(Weapon_Union)};
    Pool<Damage_Indicator>  damage_indicators{MAX_DAMAGE_INDICATORS};
    // Pool<XP_Drop>           xp_drops{MAX_XP_DROPS};
    // Wave                    wave{};
    // Pool<Countdown>         countdowns{MAX_COUNTDOWNS};
    Vec2 quad_tree_dimensions {3000,3000};
    Quad_Tree<Enemy*> enemy_quad_tree {{0,0}, quad_tree_dimensions, 5};

    void init(Vec2 screen_dim) {
        player.init();

        camera.target = {player.pos.x(), player.pos.y()};
        camera.offset = {screen_dim.x() / 2, screen_dim.y() / 2};
        camera.zoom = 0.5f;

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
        if (IsKeyDown(KEY_MINUS)) {
            camera.zoom -= 0.2f * TICK_TIME;
        }
        if (IsKeyDown(KEY_EQUAL)) {
            camera.zoom += 0.2f * TICK_TIME;
        }
        camera.target = {player.pos.x(), player.pos.y()};
    }

    void tick() {
        player.tick();

        update_camera();

        // Tick weapons
        For_Pool(weapons, it, {
            ((Weapon*)it)->tick(player, damage_zones, enemies);
        });

        //(Re)build enemy quad tree
        //TODO: Center enemy quad tree around player and not around world origin
        enemy_quad_tree.reset(player.pos, quad_tree_dimensions); // first clear the tree from the last frame
        For_Pool(enemies, it, {
            enemy_quad_tree.add_entity_quad(it, it->pos, it->dim);
        });

        // Reset enemy forces
        For_Pool(enemies, it, {
            it->force = {0,0};
        });

        separate_enemies();

        // tick enemies
        For_Pool(enemies, it, {
            it->tick(player);
        });

        // Damage_Zone-Enemy collisions
        For_Pool(damage_zones, dz, {
            if (!dz->is_active) continue;

            auto search_result = enemy_quad_tree.search(dz->pos, dz->dim);
            
            for (int l = 0; l < 4; ++l) {
                Quad_Tree_Leaf<Enemy*> *leaf = search_result.leaves[l];
                if (!leaf) continue;
                for (int j = 0; j < leaf->entity_count; ++j) {
                    Enemy *e = leaf->entities[j];
                    if (aabb_collision_check(dz->pos - dz->dim/2.0f, dz->dim, e->pos - e->dim/2.0f, e->dim)) {
                        e->health -= dz->damage;
                        e->flash_time = 10;

                        // damage indicator
                        damage_indicators.add({e->pos, (int)dz->damage});
                    }
                }
            }
        });

        // free killed enemies
        For_Pool(enemies, it, {
            if (it->health <= 0) enemies.free(it_i);
        });

        // tick damage indicators
        For_Pool(damage_indicators, dz, {
            dz->lifetime -= 1;
            if (dz->lifetime <= 0) {
                damage_indicators.free(dz_i);
            }
        });
    }

    bool aabb_collision_check(Vec2 pos0, Vec2 dim0, Vec2 pos1, Vec2 dim1) const {
        return pos0.x() < pos1.x() + dim1.x() && pos0.x() + dim0.x() > pos1.x() 
                && pos0.y() < pos1.y() + dim1.y() && pos0.y() + dim0.y() > pos1.y(); 
    }

    void separate_enemies() {
        for (int i = 0; i < enemies.capacity(); ++i) {
            Enemy *e0 = enemies.get(i);
            if (!e0) { continue; }

            //if (!is_pos_in_view(e0->pos)) { continue; } // only handle collisions for enemies in view

            Vec2 influence_zone_dim = e0->dim * 1.0f;
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
                        e0->force += repulsion * e1_to_e0;
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
        float w = enemy_quad_tree.dimensions().x();
        float h = enemy_quad_tree.dimensions().y();
        float x = enemy_quad_tree.center().x() - w/2.0f;
        float y = enemy_quad_tree.center().y() - h/2.0f;
        DrawRectangleLines(x, y, w, h, RED);
    }

    void draw() {
        BeginMode2D(camera);

            // Draw grid
            // rlPushMatrix();
            // rlTranslatef(0, 25*50, 0);
            // rlRotatef(90, 1, 0, 0);
            // DrawGrid(100, 50);
            // rlPopMatrix();

            // Draw test rect
            Vec2 rect_top_left = {100,200};
            Vec2 rect_dim = {100, 300};
            Color color = RED;
            if (aabb_collision_check(player.pos - player.dim/2.0f, player.dim, rect_top_left, rect_dim)) {
                color = GREEN;
            }
            DrawRectangle(rect_top_left.x(),rect_top_left.y(), rect_dim.x(), rect_dim.y(), color);

            // Draw entities
            player.draw();

            For_Pool(enemies, it, { it->draw(); });

            // draw weapons
            For_Pool(weapons, it, { ((Weapon*)it)->draw(); });

            // draw damage zones (debug)
            For_Pool(damage_zones, it, { it->draw(); });

            enemy_quad_tree.draw();

            // draw damage indicators
            For_Pool (damage_indicators, it, {
                int dmg = it->damage;
                DrawRectangle(it->pos.x(), it->pos.y(), 30, 10, ORANGE);
            });


        EndMode2D();
    }
};

#endif