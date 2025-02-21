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
    load_texture("res/textures/blue_gem.png", "blue_gem");
}

Texture2D get_texture(const char *name) {
    return textures[std::string{name}];
}

//
// Sounds
//
std::unordered_map<std::string, Sound> sounds {};

void load_sound(const char *path, const char *name) {
    auto sound = LoadSound(path);
    if (sound.stream.buffer == nullptr) {
        fprintf(stderr, "Couldn't load sound: %s\n", path);
    }
    sounds[std::string{name}] = sound;
}

void load_sounds() {
    load_sound("res/sounds/swing.wav", "swing");
    load_sound("res/sounds/sword-unsheathe5.wav", "sword-unsheathe5");
    load_sound("res/sounds/sword-unsheathe4.wav", "sword-unsheathe4");
    load_sound("res/sounds/sword-unsheathe3.wav", "sword-unsheathe3");
    load_sound("res/sounds/sword-unsheathe2.wav", "sword-unsheathe2");
    load_sound("res/sounds/minecraft_hit.mp3", "enemy_hit");
}

Sound get_sound(const char *name) {
    std::string name_str {name};
    if (sounds.find(name_str) == sounds.end()){
        fprintf(stderr, "get_sound: sound %s not loaded\n", name);
    }
    return sounds[name_str];
}

//
// Shaders
//

std::unordered_map<std::string, Shader> shaders {};

void load_shader(const char *path, const char *name) {
    auto shader = LoadShader(nullptr, path);
    if (shader.id == 0) {
        fprintf(stderr, "Couldn't load shader: %s\n", path);
    }
    shaders[std::string{name}] = shader;
}

void load_shaders() {
    load_shader("res/shaders/flash.fs", "flash");
}

Shader get_shader(const char *name) {
    std::string name_str {name};
    if (shaders.find(name_str) == shaders.end()){
        fprintf(stderr, "get_shader: shader %s not loaded\n", name);
    }
    return shaders[name_str];
}

//
//
//

void load_resources() {
    load_textures();
    load_sounds();
    load_shaders();
}

