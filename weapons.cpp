#include <string.h>
#include <stdio.h>
#include <new>

#include "raylib.h"

struct Pool {

    unsigned char *slots;
    int slot_size;
    int cur_slot = 0;

    Pool(int slot_count, int slot_size) : slots{ new unsigned char[slot_size * slot_count] }, slot_size{slot_size} {
        printf("Making pool with %d slots\n", slot_count);
    }

    template< typename T >
    T *add(const T &value) {
        printf("Adding to pool...\n");
        unsigned char *addr = slots + slot_size * cur_slot;
        cur_slot += 1;
        T *result = new (addr) T{value};
        return result;
    }
};

struct Weapon {
    const char *weapon_type;
    int remaining_ticks;
    bool is_cooling_down; // if false => executing attack
    int cooldown_time;
    int attack_time;
    bool on_attack_event; // if true => this tick started an attack

    Weapon(int cooldown_time, int attack_time) : cooldown_time{cooldown_time}, attack_time{attack_time} {}

    void tick() {
        printf("Weapon::tick\n");
        fire();
    }

    virtual void fire() = 0;
};

struct Whip : public Weapon {
    Whip() : Weapon{10, 20} {}

    void fire() override {
        printf("Whip fire\n");
    }
};

struct Bibles : public Weapon {
    int bible_count;

    Bibles(int bible_count) : Weapon{69, 420}, bible_count{bible_count} {}

    void fire() override {
        printf("Bibles fire\n");
    }
};

union Weapon_Union {
    Whip whip;
    Bibles bibles;
};

Pool weapons{100, sizeof(Weapon_Union)};

Weapon *make_weapon(const char* weapon_type) {
    Weapon *result = nullptr;

    if ( strcmp(weapon_type, "Whip") == 0 ) {
        result = weapons.add(Whip{});
    }
    if ( strcmp(weapon_type, "Bibles") == 0 ) {
        result = weapons.add(Bibles{3});
    }

    return result;
}

int main() {
    printf("Hello there\n");

    const char *weapon_list[] = {"Bibles", "Whip", "Bibles"};

    for (int i = 0; i < 3; ++i) {
        Weapon *weapon = make_weapon(weapon_list[i]);
        weapon->tick();
    }

    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");
    SetTargetFPS(60);

    while (!WindowShouldClose())  // Detect window close button or ESC key
    {
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText("Hello, raylib!", 190, 200, 20, LIGHTGRAY);
        EndDrawing();
    }

    CloseWindow();  // Close window and unload resources
    return 0;
}