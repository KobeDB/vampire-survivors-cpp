#ifndef RESOURCES_H
#define RESOURCES_H

#include "raylib.h"

void load_resources();

Texture2D get_texture(const char *name);

Sound get_sound(const char *name);

Shader get_shader(const char *name);

#endif