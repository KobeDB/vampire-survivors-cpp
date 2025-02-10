#ifndef PARTICLES_H
#define PARTICLES_H

#include "basic.h"

struct Particle {
    Vec2 position {};
    Vec2 velocity = {0,0};
    float rotation = 0;
    float rotation_speed = 0;
    int lifetime = 0;
    Vec2 scaling = {0,0};
    bool flip_x = false;
    float alpha = 1.0f;
    Vec3 color = ;
};

#endif