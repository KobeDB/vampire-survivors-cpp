#ifndef WEAPONS_H
#define WEAPONS_H

#include <string.h>
#include <stdio.h>

#include "raylib.h"

#include "entities.h"

#define _USE_MATH_DEFINES
#include <math.h>

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

// WARNING: don't instantiate this type, this is just for easily getting the biggest sizeof the derived Weapons
union Weapon_Union {
    Whip whip;
    Bibles bibles;
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