#include <string>
#include <cstring> // 使用 strlen() 计算文本长度
#include "raylib.h"
#include "client.h"
#include "menu.h"

char inputText[256] = "";     // 用户输入的文本
int letterCount = 0;          // 记录输入文本的长度
bool isInputActive = false;   // 是否在输入框内
int framesCounter = 0;        // 用于光标闪烁
bool isNameConfirmed = false; // 是否确认名字
char clientName[256] = "";    // 确认后的用户名

bool isClientThreadStarted = false;

bool countdownStarted = false;
float countdownTimer = 5.0f;
int prevCountdownValue = 5;

int start = 0 ;

// 检测是否点击输入框
bool clickButton(Rectangle NameButton, Sound clickSound)
{
    Vector2 mousePos = GetMousePosition();
    bool hoverName = CheckCollisionPointRec(mousePos, NameButton);

    if (isNameConfirmed) // 如果名字已确认，强制恢复默认光标
    {
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        return false; // 禁止继续输入
    }

    if (hoverName)
    {
        if (isInputActive)
        {
            SetMouseCursor(MOUSE_CURSOR_IBEAM); // 输入中时，光标为文本
        }
        else
        {
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND); // 悬停但未输入时，显示手势
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            PlaySound(clickSound);
            isInputActive = true; // 开始输入
        }
    }
    else if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        isInputActive = false; // 点击外部区域时，取消输入
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }

    return isInputActive;
}

// 处理用户输入
void handleInput()
{
    if (!isInputActive || isNameConfirmed)
        return; // 如果名字已确认，则不允许修改

    int key = GetCharPressed();
    while (key > 0)
    {
        if ((key >= 32) && (key <= 125) && (letterCount < 255))
        {
            inputText[letterCount] = (char)key;
            inputText[letterCount + 1] = '\0';
            letterCount++;
        }
        key = GetCharPressed();
    }

    // 处理退格键
    if (IsKeyPressed(KEY_BACKSPACE))
    {
        letterCount--;
        if (letterCount < 0)
            letterCount = 0;
        inputText[letterCount] = '\0';
    }

    // **处理 Enter 键**
    if (IsKeyPressed(KEY_ENTER) && letterCount > 0)
    {
        strcpy(clientName, inputText);        // 复制输入内容到 clientName
        isNameConfirmed = true;               // 确认用户名
        isInputActive = false;                // 禁止继续输入
        SetMouseCursor(MOUSE_CURSOR_DEFAULT); // **强制恢复默认光标**
    }
}

void loadingText(Font myFont, float posx, float posy)
{
    int dots = (framesCounter / 30) % 4;

    // 生成 "Loading" + ".", "..", "..."
    std::string displayText = "waiting" + std::string(dots, '.');
    DrawTextEx(myFont, displayText.c_str(), {posx, posy}, 15, 2, WHITE);
}

void drawGradientBackground()
{
    DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(), {31, 139, 102, 255}, {20, 89, 46, 255});
}

const char *getClientName()
{
    return clientName;
}

void DrawRoom()
{
    const int screenWidth = 480;
    const int screenHeight = 1000;

    if (!isClientThreadStarted)
    {
        startClientThread();
        isClientThreadStarted = true;
    }

    handleInput();                        // 用户输入
    framesCounter++;                      // 计数器增加用于光标闪烁
    std::string clientID = getClientID(); // **实时获取服务器消息**
    std::string opponent = getOpponentName();

    ClearBackground(RAYWHITE);

    // UI[Top]
    drawGradientBackground(); // 绘制渐变背景
    DrawCircle(screenWidth / 2, screenHeight * 0.20, 50, RAYWHITE);
    DrawTextEx(myFont, "Q", {static_cast<float>(screenWidth * 0.465), static_cast<float>(screenHeight * 0.173)}, 50, 0, {67, 121, 95, 255});
    DrawTextEx(myFont, "My Quoridor", {static_cast<float>(screenWidth * 0.31), static_cast<float>(screenHeight * 0.27)}, 30, 2, WHITE);
    DrawTextEx(myFont, "Welcome Back!", {static_cast<float>(screenWidth * 0.15), static_cast<float>(screenHeight * 0.40)}, 50, 1, WHITE);
    DrawTextEx(myFont, "To connect with your Friend", {static_cast<float>(screenWidth * 0.28), static_cast<float>(screenHeight * 0.46)}, 15, 1, WHITE);
    DrawTextEx(myFont, "Please Enter your Name in the Box below", {static_cast<float>(screenWidth * 0.20), static_cast<float>(screenHeight * 0.48)}, 15, 1, WHITE);

    // UI[Middle]
    Rectangle NameButton = {float(screenWidth * 0.13), float(screenHeight * 0.58), float(screenWidth * 0.75), float(screenHeight * 0.05)};
    DrawRectangleRoundedLines(NameButton, 1, 0, WHITE);
    if (!clickButton(NameButton, clickSound))
    {
        // 只在未输入时显示 "NAME"
        if (letterCount == 0)
        {
            DrawTextEx(myFont, "NAME", {NameButton.x + 145, NameButton.y + 18}, 17, 2, WHITE);
        }
    }
    DrawText(inputText, NameButton.x + 145, NameButton.y + 18, 17, WHITE); // 绘制用户输入的文本
    if (isInputActive && (framesCounter / 20) % 2 == 0)                    // 绘制光标（闪烁效果）
    {
        DrawText("_", NameButton.x + 145 + MeasureText(inputText, 17), NameButton.y + 18, 17, WHITE);
    }
    DrawTextEx(myFont, "Press 'Enter' to confirm ", {screenWidth * 0.33, screenHeight * 0.64}, 12, 2, {121, 180, 139, 255});

    int namelength = strlen(clientName); // 判断用户是否输入名字

    //  UI[Bottom]
    // 计算文本位置
    Vector2 client1Pos = {screenWidth * 0.55f, screenHeight * 0.80f};
    Vector2 client2Pos = {screenWidth * 0.55f, screenHeight * 0.83f};

    // 画 Client 1 UI
    DrawTextEx(myFont, "client 1", {screenWidth * 0.33f, screenHeight * 0.80f}, 15, 2, WHITE);
    DrawLine(screenWidth / 2, screenHeight * 0.80f, screenWidth / 2, screenHeight * 0.817f, WHITE);

    // 画 Client 2 UI
    DrawTextEx(myFont, "client 2", {screenWidth * 0.33f, screenHeight * 0.83f}, 15, 2, WHITE);
    DrawLine(screenWidth / 2, screenHeight * 0.83f, screenWidth / 2, screenHeight * 0.847f, WHITE);

    bool starting = false ;

    // 根据 clientID 放置自己的名字
    if (clientID == "1")
    {
        DrawTextEx(myFont, clientName, client1Pos, 15, 2, WHITE);
        if (opponent.empty())
        {
            loadingText(myFont, client2Pos.x, client2Pos.y);
        }
        else
        {
            DrawTextEx(myFont, opponent.c_str(), client2Pos, 15, 2, WHITE);
            starting = true ;
        }
    }
    else if (clientID == "2")
    {
        DrawTextEx(myFont, clientName, client2Pos, 15, 2, WHITE);
        if (opponent.empty())
        {
            loadingText(myFont, client1Pos.x, client1Pos.y);
        }
        else
        {
            DrawTextEx(myFont, opponent.c_str(), client1Pos, 15, 2, WHITE);
            starting = true ;
        }
    }
    else
    {
        // 未知 ID 情况
        loadingText(myFont, client1Pos.x, client1Pos.y);
        loadingText(myFont, client2Pos.x, client2Pos.y);
    }


    if(starting)
    {
        // Start the countdown when both players are connected
        if (!countdownStarted)
        {
            countdownStarted = true;
            countdownTimer = 5.0f;
            prevCountdownValue = 5;
        }

        // Update the countdown timer
        if (countdownStarted)
        {
            countdownTimer -= GetFrameTime();

            // Only display the integer part of the countdown
            int currentCountdownValue = static_cast<int>(countdownTimer);

            // Make sure it doesn't go below 0
            if (currentCountdownValue < 0)
                currentCountdownValue = 0;

            // Display the countdown value
            const char *countdownText = TextFormat("%d", currentCountdownValue);
            DrawTextEx(myFont, countdownText, {screenWidth * 0.48f, screenHeight * 0.72f}, 30, 2, RAYWHITE);

            // Add a label above the number
            DrawTextEx(myFont, "Game starting in", {screenWidth * 0.33f, screenHeight * 0.70f}, 20, 2, WHITE);

            // Check if countdown is finished
            if (countdownTimer <= 0.0f)
            {
                start = 1 ;
                countdownStarted = false;
                
            }
        }
    }
    else
    {
        // Reset countdown if the other player disconnects
        countdownStarted = false;
    }

    ScreenFadeIn();
}


int GameStart()
{
    return start ;
}