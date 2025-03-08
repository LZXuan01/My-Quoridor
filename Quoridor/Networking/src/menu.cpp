#include "menu.h"
#include <cmath>
#include <thread>

using namespace std;

Color TextBlack = {84, 74, 65, 255};
Color light_green = {31, 139, 102, 255};
// rgb(24, 109, 58)
// rgb(25, 107, 57)
// rgb(31, 139, 102)
// rgb(27, 113, 66)
// rgb(67, 121, 95)
// rgb(191, 227, 215)
// rgb(17, 158, 177)

// 资源
Font myFont;
Sound clickSound;
Sound alertSound ;

//  淡入动画
void ScreenFadeIn()
{
    float fadeSpeed = 0.4f;
    static float fadeOpacity = 1.0f;
    fadeOpacity -= fadeSpeed * GetFrameTime();
    if (fadeOpacity < 0.0f)
        fadeOpacity = 0.0f;

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade({191, 227, 215, 255}, fadeOpacity));
}

// 颜色渐变
void DrawGradientCircle(int centerX, int centerY, float radius)
{
    for (int y = -radius; y <= radius; y++)
    {
        float t = (float)(y + radius) / (2 * radius);
        Color color = {(unsigned char)(25 + t * 6), (unsigned char)(107 + t * 28), (unsigned char)(57 + t * 45), 255};
        int lineWidth = (int)(sqrt(radius * radius - y * y));
        DrawLine(centerX - lineWidth, centerY + y, centerX + lineWidth, centerY + y, color);
    }
}

// 按钮点击事件
int ClickButton()
{
    Vector2 mousePos = GetMousePosition();

    Rectangle TapButton = {float(GetScreenWidth() * 0.05), float(GetScreenHeight() * 0.62), float(GetScreenWidth() * 0.90), float(GetScreenHeight() * 0.05)};
    Rectangle ExitButton = {float(GetScreenWidth() * 0.05), float(GetScreenHeight() * 0.70), float(GetScreenWidth() * 0.90), float(GetScreenHeight() * 0.05)};

    bool hoverTap = CheckCollisionPointRec(mousePos, TapButton);
    bool hoverExit = CheckCollisionPointRec(mousePos, ExitButton);

    SetMouseCursor(hoverTap || hoverExit ? MOUSE_CURSOR_POINTING_HAND : MOUSE_CURSOR_DEFAULT);

    if (hoverTap && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        PlaySound(clickSound);
        this_thread::sleep_for(chrono::milliseconds(100));
        return 1;
    }
    if (hoverExit && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        PlaySound(clickSound);
        this_thread::sleep_for(chrono::milliseconds(100));
        return 2;
    }

    return 0;
}

// 初始化菜单
void InitMenu()
{
    InitAudioDevice();
    myFont = LoadFont("assets/COOPBL.TTF");
    clickSound = LoadSound("assets/clickSound.wav");
    alertSound = LoadSound("assets/game_alert.wav");
}

// 绘制菜单
void DrawMenu()
{
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    ClearBackground(WHITE);

    // UI [Circle]
    DrawRectangle(screenWidth / 2, screenHeight * 0.19, screenWidth / 2, screenHeight / 4, {24, 109, 58, 255});
    DrawRectangle(0, 0, screenWidth / 2, screenHeight / 5, {27, 113, 66, 255});
    DrawGradientCircle(screenWidth / 2, screenHeight * 0.20, 240);
    DrawCircle(screenWidth / 2, screenHeight * 0.20, 50, RAYWHITE);
    DrawTextEx(myFont, "Q", {static_cast<float>(screenWidth * 0.465), static_cast<float>(screenHeight * 0.173)}, 50, 0, {67, 121, 95, 255});
    DrawTextEx(myFont, "My Quoridor", {static_cast<float>(screenWidth * 0.33), static_cast<float>(screenHeight * 0.27)}, 30, 2, WHITE);

    // UI [Middle]
    DrawTextEx(myFont, "welcome", {static_cast<float>(screenWidth * 0.28), static_cast<float>(screenHeight / 2)}, 50, 5, {27, 113, 66, 255});
    DrawTextEx(myFont, "Are you ready to play the Quoridor ?", {static_cast<float>(screenWidth * 0.20), static_cast<float>(screenHeight * 0.55)}, 15, 2, TextBlack);
    
    Rectangle TapButton = {float(screenWidth * 0.05), float(screenHeight * 0.62), float(screenWidth * 0.90), float(screenHeight * 0.05)};
    DrawRectangleRounded(TapButton, 1, 0, {191, 227, 215, 255});
    
    Rectangle ExitButton = {float(screenWidth * 0.05), float(screenHeight * 0.70), float(screenWidth * 0.90), float(screenHeight * 0.05)};
    DrawRectangleRounded(ExitButton, 1, 0, {191, 227, 215, 255});

    DrawTextEx(myFont, "TAP HERE", {static_cast<float>(screenWidth * 0.40), static_cast<float>(screenHeight * 0.635)}, 20, 2, {31, 139, 102, 255});
    DrawTextEx(myFont, "EXIT", {static_cast<float>(screenWidth * 0.44), static_cast<float>(screenHeight * 0.715)}, 20, 2, {31, 139, 102, 255});
    DrawTextEx(myFont, "Don't leave me alone", {static_cast<float>(screenWidth * 0.35), static_cast<float>(screenHeight * 0.76)}, 13, 2, TextBlack);

    // UI [Bottom]
    DrawRectangleRounded({float(screenWidth * 0.36), float(screenHeight * 0.85), float(screenWidth * 0.268), float(screenHeight * 0.03)}, 1, 0, {17, 177, 133, 255});
    DrawTextEx(myFont, "DONATE", {static_cast<float>(screenWidth * 0.43), static_cast<float>(screenHeight * 0.858)}, 13, 2, WHITE);
    DrawTextEx(myFont, "https://github.com/LZXuan01", {static_cast<float>(screenWidth * 0.33), static_cast<float>(screenHeight * 0.90)}, 9, 2, light_green);

    ScreenFadeIn();
}


// 释放资源
void UnloadMenu()
{
    UnloadFont(myFont);
    UnloadSound(clickSound);
    UnloadSound(alertSound);
    CloseAudioDevice();
    
}
