#pragma once
#include "raylib.h"


// 按钮结构体
struct Button
{
    Rectangle rect;   // 按钮的矩形区域
    const char *text; // 按钮显示的文本
    Color color;      // 按钮的颜色
};

Color Board = {174, 160, 145, 255};
Color background = {244, 243, 232, 255};
Color brown = {160, 143, 128, 255};
Color white = {248, 241, 238, 255};
Color black = {84, 74, 65, 255};
Color line = {176, 159, 145, 255};
Color textcolor = {105, 103, 92, 255};

// 绘制按钮
void DrawButton(Button button)
{
    // 绘制按钮背景
    DrawRectangleRec(button.rect, button.color);
    // 计算文本的宽度和高度
    int textWidth = MeasureText(button.text, 20);
    int textHeight = 20;
    // 计算文本的位置（居中）
    int textX = button.rect.x + (button.rect.width - textWidth) / 2;
    int textY = button.rect.y + (button.rect.height - textHeight) / 2;
    // 绘制文本
    DrawText(button.text, textX, textY, 20, textcolor);
}

void showTutorial()
{
    // 初始化窗口
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Quoridor Tutorial");

    // 教学内容
    const char *tutorialText[] = {
        "Welcome to Quoridor!",
        "The goal of the game is to reach the opposite side of the board.",
        "Player 1 (White) starts at the top and aims for the right side of the board .",
        "Player 2 (Black) starts at the bottom and aims for the left side of the board.",
        "You can move your pawn or place walls to block your opponent.",
        "Press Q to rotate walls horizontally or vertically.",
        "Press ESC to return to the main menu."};
    int numLines = sizeof(tutorialText) / sizeof(tutorialText[0]);

    // 主循环
    while (!WindowShouldClose())
    {
        // 检测 ESC 键按下
        if (IsKeyPressed(KEY_ESCAPE))
        {
            break; // 退出教程
        }

        // 开始绘制
        BeginDrawing();
        ClearBackground(white); // 背景颜色为 White

        // 绘制教学内容
        int startY = 50; // 起始 Y 坐标
        for (int i = 0; i < numLines; i++)
        {
            DrawText(tutorialText[i], 50, startY + i * 30, 20, textcolor); // 字体颜色为 TextColor
        }

        // 结束绘制
        EndDrawing();
    }

    // 关闭窗口
    CloseWindow();
}

int showmenu()
{
    // 初始化窗口
    const int screenWidth = 640;
    const int screenHeight = 1000;
    InitWindow(screenWidth, screenHeight, "Quoridor Menu");

    InitAudioDevice();

    Sound clickSound = LoadSound("assets\\mixkit-modern-technology-select-3124.wav"); // 确保 click.wav 文件存在

    // 定义按钮
    Button startButton = {{220, 400, 200, 50}, "Start",brown};
    Button tutorialButton = {{220, 500, 200, 50}, "Tutorial",brown};
    Button quitButton = {{220, 600, 200, 50}, "Quit",brown};

    int result = 0; // 默认返回值

    // 主循环
    while (!WindowShouldClose())
    {
        // 获取鼠标位置
        Vector2 mousePosition = GetMousePosition();

        // 检查按钮点击
        if (CheckCollisionPointRec(mousePosition, startButton.rect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            // 点击了开始按钮
            PlaySound(clickSound);
            result = 1; // 返回 1 表示开始游戏
            break;      // 退出菜单循环
        }
        if (CheckCollisionPointRec(mousePosition, tutorialButton.rect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            // 点击了教学按钮
            PlaySound(clickSound);
            result = 2; // 返回 2 表示打开教程
            break;      // 退出菜单循环
        }
        if (CheckCollisionPointRec(mousePosition, quitButton.rect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            // 点击了退出按钮
            PlaySound(clickSound);
            result = 0; // 返回 0 表示退出
            break;      // 退出菜单循环
        }

        // 开始绘制
        BeginDrawing();
        ClearBackground(white);

        // 绘制标题
        const char *title = "Quoridor";
        int titleWidth = MeasureText(title, 50);
        DrawText(title, (screenWidth - titleWidth) / 2, 200, 50, textcolor);

        // 绘制按钮
        DrawButton(startButton);
        DrawButton(tutorialButton);
        DrawButton(quitButton);

        // 结束绘制
        EndDrawing();
    }

    UnloadSound(clickSound);

    // 关闭音频设备
    CloseAudioDevice();

    // 关闭窗口
    CloseWindow();
    return result; // 返回结果
}