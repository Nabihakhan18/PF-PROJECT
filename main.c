#include "raylib.h"   // game window, drawing, textures, audio
#include <stdlib.h>   // rand(), srand()
#include <time.h>     // time() -- used to seed the random number generator

// --- STRUCT: bundles all data for ONE mole into a single variable ---
typedef struct Mole {
    Vector2 pos;    // screen position (x, y) in pixels
    int active;     // 1 = mole is visible/poppd up, 0 = hidden
    float timer;    // counts DOWN to 0, then mole auto-hides
} Mole;

int main() {

    // --- WINDOW & AUDIO SETUP (runs once) ---
    InitWindow(800, 600, "Whack-a-Mole - PF Project (C Version)");
    InitAudioDevice();                  // must call before any audio functions
    SetTargetFPS(60);                   // lock the game to 60 frames per second
    srand(time(NULL));                  // seed RNG with current time so rand() differs every run

    // --- LOAD ASSETS ---
    Music bgMusic = LoadMusicStream("song.mp3");   // streams audio from file (more efficient than LoadSound for long tracks)
    PlayMusicStream(bgMusic);

    Texture2D moleTex = LoadTexture("mole.png");   // sprite for the mole
    Texture2D holeTex = LoadTexture("hole.png");   // sprite for the hole (always visible)

    // --- GAME STATE VARIABLES ---
    int score     = 0;
    int highScore = 0;

    int gameStarted = 0;   // 0 = on menu, 1 = game is running
    int gameOver    = 0;   // 0 = still playing, 1 = game ended

    // --- DIFFICULTY SETTINGS (changed by key press on menu) ---
    int   difficulty  = 1;
    float moleDuration = 1.0f;  // how many seconds a mole stays visible

    // --- CREATE 6 MOLES and set their fixed grid positions ---
    Mole moles[6];

    moles[0].pos = (Vector2){150, 200};   // row 1, col 1
    moles[1].pos = (Vector2){350, 200};   // row 1, col 2
    moles[2].pos = (Vector2){550, 200};   // row 1, col 3
    moles[3].pos = (Vector2){150, 350};   // row 2, col 1
    moles[4].pos = (Vector2){350, 350};   // row 2, col 2
    moles[5].pos = (Vector2){550, 350};   // row 2, col 3

    // start all moles as hidden with no timer running
    for (int i = 0; i < 6; i++) {
        moles[i].active = 0;
        moles[i].timer  = 0;
    }

    // --- SPAWN TIMER: counts up; when it hits spawnRate, pop a mole ---
    float spawnTimer = 0;
    float spawnRate  = 0.8f;   // seconds between mole spawns

    // --- ACHIEVEMENT FLAGS: once set to 1, they stay 1 ---
    int achievement10 = 0;   // unlocked at score >= 10
    int achievement25 = 0;   // unlocked at score >= 25


    // ================================================================
    // GAME LOOP: runs 60 times per second until window is closed
    // ================================================================
    while (!WindowShouldClose()) {

        UpdateMusicStream(bgMusic);   // MUST be called every frame to keep music streaming

        // ---- MENU: wait for player to pick a difficulty ----
        if (!gameStarted) {
            if (IsKeyPressed(KEY_ONE)) {
                difficulty   = 1;
                moleDuration = 1.2f;   // moles stay up longer
                spawnRate    = 1.0f;   // new mole every 1 second
                gameStarted  = 1;      // leave the menu
            }
            if (IsKeyPressed(KEY_TWO)) {
                difficulty   = 2;
                moleDuration = 0.8f;
                spawnRate    = 0.7f;
                gameStarted  = 1;
            }
            if (IsKeyPressed(KEY_THREE)) {
                difficulty   = 3;
                moleDuration = 0.5f;   // moles vanish very quickly on hard
                spawnRate    = 0.5f;   // new mole every 0.5 seconds
                gameStarted  = 1;
            }
        }

        // ---- GAMEPLAY: only runs when game is active ----
        if (gameStarted && !gameOver) {

            // -- SPAWN: add elapsed time; when enough time passes, pop a random mole --
            spawnTimer += GetFrameTime();   // GetFrameTime() = seconds since last frame (~0.016s at 60fps)

            if (spawnTimer >= spawnRate) {
                spawnTimer = 0;                // reset the stopwatch

                int index = rand() % 6;        // pick a random mole slot (0 to 5)
                moles[index].active = 1;       // make it visible
                moles[index].timer  = moleDuration;  // give it its countdown
            }

            // -- AUTO-HIDE: count each active mole's timer down; hide it when it reaches 0 --
            for (int i = 0; i < 6; i++) {
                if (moles[i].active) {
                    moles[i].timer -= GetFrameTime();   // subtract this frame's time

                    if (moles[i].timer <= 0) {
                        moles[i].active = 0;   // timer ran out → mole ducks back down
                    }
                }
            }

            // -- CLICK DETECTION: check if the mouse click landed on any active mole --
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 mouse = GetMousePosition();   // pixel coords of the click

                for (int i = 0; i < 6; i++) {
                    if (moles[i].active) {

                        // build a bounding rectangle around this mole using its position + texture size
                        Rectangle moleRect = {
                            moles[i].pos.x,
                            moles[i].pos.y,
                            (float)moleTex.width,
                            (float)moleTex.height
                        };

                        // did the click land inside the mole's rectangle?
                        if (CheckCollisionPointRec(mouse, moleRect)) {
                            moles[i].active = 0;   // whacked! hide the mole immediately
                            score++;

                            // update high score in real time
                            if (score > highScore)
                                highScore = score;

                            // unlock achievements (flags stay 1 once set)
                            if (score >= 10) achievement10 = 1;
                            if (score >= 25) achievement25 = 1;
                        }
                    }
                }
            }

            // -- ESC → go to game-over screen --
            if (IsKeyPressed(KEY_ESCAPE)) {
                gameOver = 1;
            }
        }

        // ---- RESTART: on game-over screen, R resets everything ----
        if (gameOver && IsKeyPressed(KEY_R)) {
            score         = 0;
            gameOver      = 0;
            gameStarted   = 0;      // return to the menu
            achievement10 = 0;
            achievement25 = 0;

            // clear all mole states so nothing is left over from last game
            for (int i = 0; i < 6; i++) {
                moles[i].active = 0;
                moles[i].timer  = 0;
            }
        }


        // ================================================================
        // DRAWING: everything between BeginDrawing/EndDrawing appears on screen
        // ================================================================
        BeginDrawing();
        ClearBackground(RAYWHITE);   // wipe last frame

        // -- MENU SCREEN --
        if (!gameStarted) {
            DrawText("WHACK-A-MOLE", 230, 100, 45, DARKGREEN);
            DrawText("Press 1 for EASY",   250, 250, 30, BLACK);
            DrawText("Press 2 for MEDIUM", 250, 300, 30, BLACK);
            DrawText("Press 3 for HARD",   250, 350, 30, BLACK);
        }
        // -- GAME OVER SCREEN --
        else if (gameOver) {
            DrawText("GAME OVER!", 260, 200, 50, RED);
            DrawText(TextFormat("Score: %d",      score),     320, 260, 30, BLACK);
            DrawText(TextFormat("High Score: %d", highScore), 285, 300, 30, BLACK);
            DrawText("Press R to Restart", 280, 360, 30, DARKGRAY);
        }
        // -- GAMEPLAY SCREEN --
        else {
            for (int i = 0; i < 6; i++) {
                // hole is always drawn at the mole's position
                DrawTexture(holeTex, moles[i].pos.x, moles[i].pos.y, WHITE);

                // mole sprite is drawn 30px ABOVE the hole to look like it's popping out
                if (moles[i].active)
                    DrawTexture(moleTex, moles[i].pos.x, moles[i].pos.y - 30, WHITE);
            }

            // live score in the top-left corner
            DrawText(TextFormat("Score: %d", score), 20, 20, 30, BLACK);

            // achievement banners (only show once unlocked)
            if (achievement10)
                DrawText("Achievement: Mole Beginner (10)", 20, 70, 20, DARKGREEN);
            if (achievement25)
                DrawText("Achievement: Mole Master (25)",   20, 95, 20, MAROON);
        }

        EndDrawing();   // swap the back buffer to the screen
    }


    // --- CLEANUP: release every resource in reverse load order ---
    UnloadTexture(moleTex);
    UnloadTexture(holeTex);
    UnloadMusicStream(bgMusic);
    CloseAudioDevice();
    CloseWindow();

    return 0;   // 0 = program finished successfully
}
