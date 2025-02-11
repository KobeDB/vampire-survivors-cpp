#ifndef PARTICLES_H
#define PARTICLES_H

#include "raylib.h"

#include "my_raylib_helpers.h"
#include "constants.h"
#include "basic.h"

struct Particle {
    Vec2 position {};
    Vec2 velocity = {0,0};
    float rotation = 0;
    float rotation_speed = 0;
    int start_lifetime = 100;
    int rem_lifetime = start_lifetime;
    Vec2 scaling = {1,1};
    bool flip_x = false;
    float alpha = 1.0f;
    Vec3 start_color = {1,1,1};
    Vec3 end_color = start_color;
    Vec3 color = start_color;

    Particle(int lifetime, Vec3 start_color, Vec3 end_color) : start_lifetime{lifetime}, rem_lifetime{start_lifetime}, start_color{start_color}, end_color{end_color}, color{start_color} {}
};

struct Particle_Emitter {
    Pool<Particle> particles {1000};
    Texture2D texture;

    Particle_Emitter() : texture{} {}

    Particle_Emitter(Texture2D texture) : texture{texture} {}
    Particle_Emitter(int max_particle_count, Texture2D texture) : particles{ max_particle_count }, texture{texture} {}

    void tick() {
        for (int i = 0; i < particles.capacity(); ++i) {
            Particle *p = particles.get(i);
            if (!p) { continue; }

            p->position += p->velocity * TICK_TIME;
            p->rotation += p->rotation_speed * TICK_TIME;
            --p->rem_lifetime;
            p->alpha = float(p->rem_lifetime) / float(p->start_lifetime);
            p->color = p->alpha * p->start_color + (1.0f-p->alpha) * p->end_color;

            if (p->rem_lifetime <= 0) {
                particles.free(i);
            }
        }
    }

    void emit(const Particle &particle) {
        particles.add(particle);
    }

    void draw() const {
        if (texture.id == 0) { return; }

        for (int i = 0; i < particles.capacity(); ++i) {
            Particle *p = particles.get(i);
            if (!p) { continue; }

            Vec2 src_rec_dim = {float(texture.width), float(texture.height)};
            if (p->flip_x) { src_rec_dim.x() = -src_rec_dim.x(); }
            Rectangle src_rec = { 0, 0, src_rec_dim.x(), src_rec_dim.y() };

            Vec2 dest_rec_dim = {50 * p->scaling.x(), 50 * p->scaling.y()};
            Rectangle dest_rec = { p->position.x(), p->position.y(), dest_rec_dim.x(), dest_rec_dim.y() };

            Vec2 origin = dest_rec_dim / 2.0f;

            auto faded = Fade(to_rl_color(p->color), p->alpha);
            DrawTexturePro(texture, src_rec, dest_rec, {origin.x(), origin.y()}, p->rotation, faded);
        }
    }
};

#endif