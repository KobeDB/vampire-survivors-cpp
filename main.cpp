#include "raylib.h"

#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "basic.h"
#include "constants.h"
#include "level.h"

#include "pool.h"

#include "resources.h"

int main() {
    printf("Hello there\n");

    Vec2 screen_dim{ 1600, 900 };

    InitWindow(screen_dim.x(), screen_dim.y(), "raylib [core] example - basic window");
    SetTargetFPS(TICKS_PER_SECOND);

    InitAudioDevice();

    auto music = LoadMusicStream("res/sounds/vampire_jam.mp3");
    PlayMusicStream(music);

    //
    // Init rand
    //
    srand(time(nullptr));

    //
    // Init resources
    //
    load_resources();

    //
    // Init game state
    //
    Level level{};
    level.init(screen_dim);

    //
    // Roemmel
    //
    Pool<Vec2> vectors{10};
    vectors.add({69,420});
    vectors.add({21, 42});

    for(int i = 0; i < vectors.capacity(); ++i) {
        auto v = vectors.get(i);
        if (!v) { continue; }
        printf("vectors[%d]=%f\n", i, v->x());
    }

    vectors.free(0);

    for(int i = 0; i < vectors.capacity(); ++i) {
        auto v = vectors.get(i);
        if (!v) { continue; }
        printf("vectors[%d]=%f\n", i, v->x());
    }

    while (!WindowShouldClose()) {
        //
        // Tick
        //
        level.tick();

        //
        // Update Music
        //
        // UpdateMusicStream(music);

        //
        // Draw
        //
        BeginDrawing();
            ClearBackground(GRAY);
            DrawText("Hello, raylib!", 190, 200, 20, LIGHTGRAY);

            level.draw();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}