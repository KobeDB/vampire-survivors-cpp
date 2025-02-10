#ifndef WEAPONS_H
#define WEAPONS_H

#include <string.h>
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include "constants.h"

#include "raylib.h"

#include "entities.h"

#include "array.h"

struct Damage_Zone {
    Vec2 pos {};
    Vec2 dim {};
    float damage {};
    Color color {};
    bool is_active {};
    int enemy_hit_count {};

    void draw() const {
        if (!is_active) { return; }
        auto corner = pos - dim/2;
        DrawRectangleLines( corner.x, corner.y, dim.x, dim.y, color);
    }
};

struct Weapon {
    const char *weapon_type {};
    int remaining_ticks {};
    bool is_cooling_down {}; // if false => executing attack
    int cooldown_time;
    int attack_time;
    bool on_attack_event {}; // if true => this tick started an attack

    Weapon(int cooldown_time, int attack_time) : cooldown_time{cooldown_time}, attack_time{attack_time} {}
    virtual ~Weapon() = default;

    void tick(const Player &player, Pool<Damage_Zone> &damage_zones, const Pool<Enemy> &enemies) {
        --remaining_ticks;
        if (remaining_ticks <= 0) {
            if (is_cooling_down) {
                on_attack_event = true; // was cooling down, now going to attack
            }
            remaining_ticks = is_cooling_down ? attack_time : cooldown_time;
            is_cooling_down = !is_cooling_down; // flip state
        }

        progress_attack(player, damage_zones, enemies);

        // turn off on-attack event
        on_attack_event = false;
    }

    virtual void progress_attack(const Player &player, Pool<Damage_Zone> &damage_zones, const Pool<Enemy> &enemies) = 0;
};

struct Whip : public Weapon {
    Pool_Handle<Damage_Zone> dz_handle {};

    Whip(Pool<Damage_Zone> &damage_zones) : Weapon{100, 10} {
        Damage_Zone the_dz {};
        the_dz.dim = {200,100};
        the_dz.damage = 50;
        the_dz.color = PINK;

        dz_handle = damage_zones.add(the_dz);
    }

    void progress_attack(const Player &player, Pool<Damage_Zone> &damage_zones, const Pool<Enemy> &enemies) override {
        auto dz = get(dz_handle);

        if (on_attack_event) {
            // Set damage zone's position
            if (player.facing_dir.x >= 0) {
                dz->pos.x = player.pos.x + player.dim.x/2 + dz->dim.x/2;
            }
            else {
                dz->pos.x = player.pos.x - player.dim.x/2 - dz->dim.x/2;
            }

            dz->pos.y = player.pos.y;
        }

        dz->is_active = !is_cooling_down;
    }
};

#define BIBLES_COOLDOWN 200
#define BIBLES_LIFETIME 500

struct Bibles : public Weapon {
    Pool_Handle<Damage_Zone> bibles[10] {};
    int bible_count;

    float revolutions_per_attack_period = 3;
    float radius = 100;

    Bibles(int bible_count, Pool<Damage_Zone> &damage_zones) : Weapon{BIBLES_COOLDOWN, BIBLES_LIFETIME}, bible_count{bible_count} {
        for (int i = 0; i < bible_count; ++i) {
            Damage_Zone bible {};
            bible.dim = {50, 75};
            bible.damage = 30;
            bible.color = BLUE;
            bibles[i] = damage_zones.add(bible);
        }
    }

    void progress_attack(const Player &player, Pool<Damage_Zone> &damage_zones, const Pool<Enemy> &enemies) override {
        for (int i = 0; i < bible_count; ++i) {
            // Update bible's damage zone
            Damage_Zone *bible = get(bibles[i]);
            bible->pos = calc_bible_center(i, player.pos, remaining_ticks);
            bible->is_active = !is_cooling_down;
        }
    }

    Vec2 calc_bible_center(int bible, Vec2 orbit_center, int rem_lifetime) const {
        float angle_between_bibles = 2 * M_PI / float(bible_count);
        float anim_progress = float(attack_time-rem_lifetime) / float(attack_time);
        float angle_offset = revolutions_per_attack_period * 2 * M_PI * anim_progress;
        float bible_angle = angle_between_bibles * float(bible) + angle_offset;
        Vec2 bible_center_on_unit_circle = {cosf(bible_angle), sinf(bible_angle)};
        Vec2 bible_center = bible_center_on_unit_circle * radius + orbit_center;
        return bible_center;
    }
};

struct Projectile {
    Pool_Handle<Damage_Zone> dz {};
    int lifetime {};
    Vec2 velocity {};
    Vec2 acceleration {};
    float rotation {};
    float rotation_speed {};
    int health = 9999;
};

struct Projectile_Weapon : public Weapon {
    Pool<Projectile> projectiles {300};
    int ticks_until_next_shot {};
    int pending_shots {};

    // constants
    int shot_count {};
    int ticks_between_shots {};

    Projectile_Weapon(int cooldown_time, int shot_count, int ticks_between_shots) : Weapon{cooldown_time, (shot_count-1)*ticks_between_shots}, shot_count{shot_count}, ticks_between_shots{ticks_between_shots} {}

    void progress_attack(const Player &player, Pool<Damage_Zone> &damage_zones, const Pool<Enemy> &enemies) override {
        if (on_attack_event) {
            pending_shots = shot_count;
        }

        --ticks_until_next_shot;

        if (pending_shots > 0 && ticks_until_next_shot <= 0) {
            --pending_shots;
            ticks_until_next_shot = ticks_between_shots;
            fire_projectiles(player, damage_zones, enemies);
        }

        for (int i = 0; i < projectiles.capacity(); ++i) {
            Projectile *proj = projectiles.get(i);
            if (!proj) { continue; }

            Damage_Zone *dz = get(proj->dz);

            --proj->lifetime;
            if (proj->lifetime <= 0 || dz->enemy_hit_count >= proj->health ) {
                // first free the projectile's Damage_Zone
                free(proj->dz);
                // free the projectile itself
                projectiles.free(i);
                // this projectile is now dead, continue to next one
                continue;
            }

            // update projectile's movement
            proj->velocity += proj->acceleration * TICK_TIME;
            dz->pos += proj->velocity * TICK_TIME;
            proj->rotation += proj->rotation_speed * TICK_TIME;
        }

    }

    virtual void fire_projectiles(const Player &player, Pool<Damage_Zone> &damage_zones, const Pool<Enemy> &enemies) = 0;

};

#define CROSS_COOLDOWN 200
#define CROSS_TICKS_BETWEEN_SHOTS 5

struct Cross : public Projectile_Weapon {
    Cross(Pool<Damage_Zone> &damage_zones) : Projectile_Weapon{CROSS_COOLDOWN, 2, CROSS_TICKS_BETWEEN_SHOTS} {}

    void fire_projectiles(const Player &player, Pool<Damage_Zone> &damage_zones, const Pool<Enemy> &enemies) override {
        // Array<Enemy_Distance_Pair> enemy_distances = find_nearest_enemies(player, enemies);
        // defer (enemy_distances.destroy());
        // if (false) {}
    }
};

struct Enemy_Distance {
    int enemy_pool_index {};
    float dist {};
    float health {};
};

// // TODO: allocate result on a temp arena allocator somehow
// Array<Enemy_Distance> find_nearest_enemies(Vec2 player_pos, Pool<Enemy> &enemies) {
//     return {};
// }

// WARNING: don't instantiate this type, this is just for easily getting the biggest sizeof the derived Weapons
union Weapon_Union {
    Whip whip;
    Bibles bibles;
    Cross cross;
    ~Weapon_Union() {} // we never instantiate Weapon_Union, but this is just to satisfy compiler
};

// Weapon *make_weapon(const char* weapon_type) {
//     Weapon *result = nullptr;

//     if ( strcmp(weapon_type, "Whip") == 0 ) {
//         result = weapons.add(Whip{});
//     }
//     // if ( strcmp(weapon_type, "Bibles") == 0 ) {
//     //     result = weapons.add(Bibles{3});
//     // }

//     return result;
// }

#endif