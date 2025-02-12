#ifndef WEAPONS_H
#define WEAPONS_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include "constants.h"

#include "raylib.h"

#include "entities.h"
#include "particles.h"

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
        Vec2 corner = pos - dim/2;
        DrawRectangleLines(corner.x(), corner.y(), dim.x(), dim.y(), color);
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

    virtual void draw() = 0;
};

struct Whip : public Weapon {
    Pool_Handle<Damage_Zone> dz_handle {};
    Particle_Emitter emitter {get_texture("slash")};

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
            if (player.facing_dir.x() >= 0) {
                dz->pos.x() = player.pos.x() + player.dim.x()/2 + dz->dim.x()/2;
            }
            else {
                dz->pos.x() = player.pos.x() - player.dim.x()/2 - dz->dim.x()/2;
            }

            dz->pos.y() = player.pos.y();

            // Emit slash particle
            Particle p {60, Vec3{1,0,0}, Vec3{1,0,0}};
            p.scaling = {7, 3};
            p.position = dz->pos;
            p.flip_x = player.facing_dir.x() < 0;
            emitter.emit(p);

            // Play slash sound
            PlaySound(get_sound("swing"));
        }

        dz->is_active = !is_cooling_down;

        emitter.tick();
    }

    void draw() override {
        emitter.draw();
    }
};

#define BIBLES_COOLDOWN 200
#define BIBLES_LIFETIME 500

struct Bibles : public Weapon {
    Pool_Handle<Damage_Zone> bibles[10] {};
    int bible_count;

    float revolutions_per_attack_period = 3;
    float radius = 100;

    float bible_scaling = 1.0f;

    Particle_Emitter emitter {get_texture("bible")};

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

        // update bibles' damage zones
        for (int i = 0; i < bible_count; ++i) {
            Damage_Zone *bible = get(bibles[i]);
            bible->pos = calc_bible_center(i, player.pos, remaining_ticks);
            bible->is_active = !is_cooling_down;
        }

        // update bible scaling
        // bibles grow at the start of the attack and shrink at the end
        float attack_progress = float(attack_time - remaining_ticks) / float(attack_time);
        if (attack_progress < 0.1f) {
            bible_scaling = attack_progress/0.1f;
        }
        else if (attack_progress > 0.9f) {
            bible_scaling = (1.0f-attack_progress)/0.1f;
        }
        else {
            bible_scaling = 1.0f;
        }

        // spawn particles
        if (!is_cooling_down) {
            for (int i = 0; i < bible_count; ++i) {
                Damage_Zone *bible = get(bibles[i]);

                Particle p {20, Vec3{1,0,1}, Vec3{1,0,0}};
                p.position = bible->pos;
                p.scaling = Vec2{0.8,0.8} * bible_scaling;
                Vec2 r = player.pos - bible->pos;
                p.velocity = player.velocity + normalize(Vec2{-r.y(), r.x()}) * 10.0f;

                emitter.emit(p);
            }
        }

        emitter.tick();
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

    void draw() override {
        emitter.draw();
        if (!is_cooling_down) {
            for (int i = 0; i < bible_count; ++i) {
                Damage_Zone *bible = get(bibles[i]);
                draw_texture(get_texture("bible"), bible->pos, bible_scaling);
            }
        }
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

    Vec2 position() const { return get(dz)->pos; }
};

struct Projectile_Weapon : public Weapon {
    Pool<Projectile> projectiles {300};
    int ticks_until_next_shot {};
    int pending_shots {};

    Particle_Emitter emitter {};
    int particle_spawn_interval {};

    // constants
    int shot_count {};
    int ticks_between_shots {};

    Projectile_Weapon(int cooldown_time, int shot_count, int ticks_between_shots) : Weapon{cooldown_time, (shot_count-1)*ticks_between_shots}, shot_count{shot_count}, ticks_between_shots{ticks_between_shots} {}

    Projectile_Weapon(int cooldown_time, int shot_count, int ticks_between_shots, Texture2D particle_tex, int particle_spawn_interval) : Weapon{cooldown_time, (shot_count-1)*ticks_between_shots}, emitter{particle_tex}, particle_spawn_interval{particle_spawn_interval}, shot_count{shot_count}, ticks_between_shots{ticks_between_shots} {}

    Projectile_Weapon(int cooldown_time, int shot_count, int ticks_between_shots, Texture2D particle_tex, int particle_spawn_interval, int particle_pool_size) : Weapon{cooldown_time, (shot_count-1)*ticks_between_shots}, emitter{particle_pool_size, particle_tex}, particle_spawn_interval{particle_spawn_interval}, shot_count{shot_count}, ticks_between_shots{ticks_between_shots} {}

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

            // emit particles
            if (particle_spawn_interval != 0) {
                if ((proj->lifetime % particle_spawn_interval) == 0) {
                    spawn_particles(*proj);
                }
            }
        }

        emitter.tick();
    }

    virtual void fire_projectiles(const Player &player, Pool<Damage_Zone> &damage_zones, const Pool<Enemy> &enemies) = 0;

    virtual void spawn_particles(const Projectile &projectile) = 0;
};

//
// ---------------------
struct Enemy_Distance {
    int enemy_pool_index {};
    float dist {};
    float health {};
};

inline int enemy_distance_comp(const void *a, const void *b) {
    return ((Enemy_Distance*)a)->dist - ((Enemy_Distance*)b)->dist;
}

// TODO: allocate result on a temp arena allocator somehow
inline Array<Enemy_Distance> find_nearest_enemies(Vec2 player_pos, const Pool<Enemy> &enemies) {

    Array<Enemy_Distance> result {};

    for (int i = 0; i < enemies.capacity(); ++i) {
        Enemy *enemy = enemies.get(i);
        if (!enemy) { continue; }
        float dist = length(enemy->pos - player_pos);
        result.push({i, dist, enemy->health});
    }

    // qsort safety: result is an array of POD Enemy_Distance elements so using qsort is fine.
    qsort(result.data(), result.size(), sizeof(result[0]), enemy_distance_comp);

    return result;
}
// ---------------------

#define MAGIC_WAND_COOLDOWN 200
#define MAGIC_WAND_TICKS_BETWEEN_SHOTS 5
#define MAGIC_WAND_DAMAGE 100

struct Magic_Wand : public Projectile_Weapon {
    int projectile_count = 10;

    Magic_Wand(Pool<Damage_Zone> &damage_zones) : Projectile_Weapon{MAGIC_WAND_COOLDOWN, 1, MAGIC_WAND_TICKS_BETWEEN_SHOTS, get_texture("flare"), 5} {}

    void fire_projectiles(const Player &player, Pool<Damage_Zone> &damage_zones, const Pool<Enemy> &enemies) override {
        Array<Enemy_Distance> enemy_distances = find_nearest_enemies(player.pos, enemies);
        defer (enemy_distances.destroy());
        int targeted_enemy = 0; // index in enemy_distances array

        for (int i = 0; i < projectile_count; ++i) {
            Damage_Zone dz {};
            dz.pos = player.pos;
            dz.dim = {20, 20};
            dz.damage = MAGIC_WAND_DAMAGE;
            dz.color = SKYBLUE;
            dz.is_active = true;
            auto dz_handle = damage_zones.add(dz);

            Projectile proj {};
            proj.dz = dz_handle;
            proj.lifetime = 420;
            proj.health = 1;

            while(targeted_enemy<enemy_distances.size() && enemy_distances[targeted_enemy].health<=0) {
                ++targeted_enemy;
            }

            Vec2 shoot_dir {};
            if (targeted_enemy < enemy_distances.size()) {
                int target_index = enemy_distances[targeted_enemy].enemy_pool_index;
                Enemy *target = enemies.get(target_index);
                assert(target);
                shoot_dir = normalize(target->pos - player.pos);
                // "simulate" damaging the enemy
                enemy_distances[targeted_enemy].health -= dz.damage;
            } else {
                shoot_dir = random_unit_vec<2>();
            }

            proj.velocity = shoot_dir * 200;

            projectiles.add(proj);
        }
    }

    void spawn_particles(const Projectile &projectile) override {
        Particle p {100, Vec3{0,0,1}, Vec3{0,1,0}};
        p.position = get(projectile.dz)->pos;
        emitter.emit(p);
    }

    void draw() override {
        emitter.draw();
    }
};

struct Cross : public Projectile_Weapon {
    Cross() : Projectile_Weapon{200, 2, 10, get_texture("cross"), 10} {}

    void fire_projectiles(const Player &player, Pool<Damage_Zone> &damage_zones, const Pool<Enemy> &enemies) override {
        Damage_Zone dz {};
        dz.pos = player.pos;
        dz.dim = {75, 75};
        dz.damage = 50;
        dz.color = YELLOW;
        dz.is_active = true;
        auto dz_handle = damage_zones.add(dz);

        Projectile proj {};
        proj.dz = dz_handle;
        proj.lifetime = 300;
        proj.rotation_speed = 100;

        Array<Enemy_Distance> enemy_distances = find_nearest_enemies(player.pos, enemies);
        defer (enemy_distances.destroy());

        Vec2 shoot_dir {};
        if (enemy_distances.size() > 0) {
            int target_index = enemy_distances[0].enemy_pool_index;
            Enemy *target = enemies.get(target_index);
            assert(target);
            shoot_dir = normalize(target->pos - player.pos);
        } else {
            shoot_dir = random_unit_vec<2>();
        }

        proj.velocity = shoot_dir * 300;

        proj.acceleration = -shoot_dir * 500;

        projectiles.add(proj);

        // play sound effect
        PlaySound(get_sound("sword-unsheathe2"));
    }

    void spawn_particles(const Projectile &projectile) override {
        Vec3 particle_color = Vec3{1,1,0.4f} * 0.6f;
        Particle p {60, particle_color, particle_color};
        p.position = get(projectile.dz)->pos;
        p.rotation_speed = 100;
        p.scaling = {2,2};
        emitter.emit(p);
    }

    void draw() override {
        emitter.draw();
        for (int i = 0; i < projectiles.capacity(); ++i) {
            Projectile *proj = projectiles.get(i);
            if (!proj) { continue; }
            draw_texture(get_texture("cross"), proj->position(), 2.0f, proj->rotation);
        }
    }

};

#define FIRE_WAND_COOLDOWN 200
#define FIRE_WAND_TICKS_BETWEEN_SHOTS 1
#define FIRE_WAND_PARTICLE_SPAWN_INTERVAL 5

struct Fire_Wand : public Projectile_Weapon {

    int fire_ball_count = 10;

    Fire_Wand() : Projectile_Weapon{FIRE_WAND_COOLDOWN, 1, FIRE_WAND_TICKS_BETWEEN_SHOTS, get_texture("fireball"), FIRE_WAND_PARTICLE_SPAWN_INTERVAL, 10000} {}

    void fire_projectiles(const Player &player, Pool<Damage_Zone> &damage_zones, const Pool<Enemy> &enemies) override {

        // shoot at random enemy
        Array<int> living_enemies {};
        defer (living_enemies.destroy());
        for (int i = 0; i < enemies.capacity(); ++i) {
            Enemy *enemy = enemies.get(i);
            if (!enemy) { continue; }
            living_enemies.push(i);
        }

        Vec2 shoot_dir {};
        if (living_enemies.size() > 0) {
            int target_index = random_int(0, living_enemies.size());
            Enemy *target = enemies.get(target_index);
            assert(target);
            shoot_dir = normalize(target->pos - player.pos);
        } else {
            shoot_dir = random_unit_vec<2>();
        }

        float spread_angle = 0.4f;
        float angle_step = spread_angle / float(fire_ball_count);
        float start_angle = -angle_step * float(fire_ball_count/2);
        for (int i = 0; i < fire_ball_count; ++i) {
            float angle = start_angle + i * angle_step;
            Vec2 proj_shoot_dir = rotate(shoot_dir, angle);

            Damage_Zone dz {};
            dz.pos = player.pos;
            dz.dim = {40, 40};
            dz.damage = 50;
            dz.color = ORANGE;
            dz.is_active = true;
            auto dz_handle = damage_zones.add(dz);

            Projectile proj {};
            proj.dz = dz_handle;
            proj.lifetime = 300;
            proj.velocity = proj_shoot_dir * 300;
            proj.rotation_speed = 200.0f;

            projectiles.add(proj);
        }
    }

    void spawn_particles(const Projectile &projectile) override {

        int angles = 3;
        float angle_step = 2 * M_PI / float(angles);
        int radius_steps = 3;
        // float dim_x = get(projectile.dz)->dim.x();
        // float radius_step = dim_x  / float(radius_steps);
        float radius_step = 0.8f * get(projectile.dz)->dim.x() / float(radius_steps);

        float max_lifetime = 50.0f;
        float min_lifetime = 5.0f;
        float max_radius = (radius_steps-1) * radius_step;

        for (int i = 0; i < angles; ++i) {
            float angle = i * angle_step;
            for (int j = 0; j < radius_steps; ++j) {
                float radius = j * radius_step;
                Vec2 particle_pos = get(projectile.dz)->pos + Vec2{cosf(angle), sinf(angle)} * radius;
                int lifetime = int(max_lifetime - (max_lifetime - min_lifetime) * (radius/max_radius));

                Vec3 particle_color = Vec3{1,1,1};
                Particle p {lifetime, particle_color, particle_color};
                p.position = particle_pos;
                p.velocity = -projectile.velocity * 0.2f;
                p.scaling = {0.2f,0.2f};

                float deviation_angle = random_float(-0.2f,0.2f);
                p.velocity = rotate(p.velocity, deviation_angle);

                float size_deviation = random_float(0.5f, 1.6f);
                p.scaling *= size_deviation;

                emitter.emit(p);
            }
        }
    }

    void draw() override {
        emitter.draw();
        for (int i = 0; i < projectiles.capacity(); ++i) {
            Projectile *projectile = projectiles.get(i);
            if (!projectile) { continue; }
            Damage_Zone *dz = get(projectile->dz);
            float scale = 1.0f;
            draw_texture(get_texture("fireball"), dz->pos, scale, projectile->rotation);
        }
    }
};

// WARNING: don't instantiate this type, this is just for easily getting the biggest sizeof the derived Weapons
union Weapon_Union {
    Whip whip;
    Bibles bibles;
    Magic_Wand magic_wand;
    Cross cross;
    Fire_Wand fire_wand;
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