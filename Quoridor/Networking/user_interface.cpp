#include "raylib.h"
#include "menu.h"
#include "game.h"
// Define possible game states
enum GameState
{
    MENU_STATE,
    ROOM_STATE,
    GAME_STATE,
    VICTORY_STATE
};


int main()
{
    int screenWidth = 480;
    int screenHeight = 1000;

    InitWindow(screenWidth, screenHeight, "Quoridor");
    InitMenu();
    SetTargetFPS(60);

    // Initialize game state
    GameState currentState = MENU_STATE;
    int winner = 0; // 0: 无胜利者, 1: 玩家1胜利, 2: 玩家2胜利

    while (!WindowShouldClose())
    {
        if (currentState == GAME_STATE)
        {
            winner = Game();
            if (winner == 1 || winner == 2)
            {
                currentState = VICTORY_STATE; // 切换到胜利界面
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // State machine
        switch (currentState)
        {
        case MENU_STATE:
        {
            DrawMenu();
            int clickResult = ClickButton();
            if (clickResult == 1)
            {
                // Transition to room state
                currentState = ROOM_STATE;
            }
            else if (clickResult == 2)
            {
                // Exit game
                CloseWindow() ;
                break;
            }
            break;
        }

        case ROOM_STATE:
        {
            DrawRoom();
            int start_the_game = GameStart();
            if (start_the_game == 1)
            {
                // Transition to game state
                currentState = GAME_STATE;
            }
            break;
        }

        case GAME_STATE:
        {
            // Clear previous screen before drawing game
            ClearBackground(RAYWHITE);

            DrawGame();
            break;
        }

        case VICTORY_STATE:
        {
            DrawVictory(winner);
            int clickResult = ClickButton();
            if (clickResult == 1)
            {
                ResetGame();
                currentState = GAME_STATE;
            }
            else if (clickResult == 2)
            {
                // Exit game
                CloseWindow();
                break;
            }
            break;
        }
        }

        EndDrawing();
    }

    UnloadMenu();
    CloseWindow();
    return 0;
}