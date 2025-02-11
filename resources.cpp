#include <unordered_map>
#include <string>
#include <stdio.h>
#include "raylib.h"

// TODO: later replace STL hash map with my own or seek alternative way
std::unordered_map<std::string, Texture2D> textures{};

void load_texture(const char *path, const char *name) {
    auto texture = LoadTexture(path);
    if (texture.id == 0) {
        fprintf(stderr, "Couldn't load texture: %s\n", path);
    }
    textures[std::string{name}] = texture;
}

void load_textures() {
    load_texture("res/textures/scarfy.png", "scarfy");
    load_texture("res/textures/skeleton.png", "skeleton");
    load_texture("res/textures/bat.png", "bat");
    load_texture("res/textures/strong_bat.png", "strong_bat");
    load_texture("res/textures/zombie.png", "zombie");
    load_texture("res/textures/bible.png", "bible");
    load_texture("res/textures/slash.png", "slash");
    load_texture("res/textures/flare.png", "flare");
    load_texture("res/textures/cross.png", "cross");
    load_texture("res/textures/fireball.png", "fireball");
}

Texture2D get_texture(const char *name) {
    return textures[std::string{name}];
}

void load_resources() {
    load_textures();
}