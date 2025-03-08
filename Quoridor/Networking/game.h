#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <vector>

extern int winner ;

int  Game();
void DrawGame();
void DrawVictory(int winner);
void ResetGame();

#endif 