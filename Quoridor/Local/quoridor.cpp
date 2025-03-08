#include "raylib.h"
#include <vector>
#include <cstdio>
#include <queue>
#include <unordered_set>
#include <unistd.h>

Color Board = {174, 160, 145, 255};         // 浅可可色（棋盘）
Color background = {244, 243, 232, 255};    // 白色（背景）
Color brown = {160, 143, 128, 255};         // 可可色
Color white = {248, 241, 238, 255};         // 白色
Color black = {84, 74, 65, 255};            // 黑色
Color line = {176, 159, 145, 255};          // 可可色（美观线条）
Color textcolor = {105, 103, 92, 255};      // 可可色（字体）
Color PrelookWallColor = {0, 228, 48, 255}; // 青色（预视墙壁）

const int boardSize = 9; // 棋盘的尺寸
const int cellSize = 60; // 每个格子的大小

const int uiHorizon = 50;   // 左右间距
const int uiVertical = 220; // 上下间距

const char *placementErrorMsg = nullptr; // 提醒用户放置墙壁错误

struct Player // 玩家结构体
{
    int x, y;    // 玩家位置
    Color color; // 玩家颜色
    int walls;   // 玩家持有的墙壁数量
};

struct Wall // 墙壁结构体
{
    int x, y;        // 墙壁的起始位置
    bool horizontal; // 墙壁方向：true为水平
    int playerid;    // 玩家回合标准
};


// 棋盘函数
void DrawBoard(); // 绘制棋盘
void DrawPosition(); // 绘制坐标

// 玩家函数
void DrawPlayer(Player player);                                             // 绘制玩家
void DrawValidMoves(Vector2 validMoves[], int validMovesCount);             // 绘制可选路径（黄色小点）
bool IsMouseOnPlayer(int mouseX, int mouseY, Player player);                // 检查是否点击到玩家角色
bool IsPathBlockedForPlayer(Player player, const std::vector<Wall> &walls); // 检查玩家放置的墙壁是否阻挡所有路径

// 墙壁函数
void DrawWalls(const std::vector<Wall> &walls);                                                 // 绘制墙壁
void DrawWallCount(Player player1, Player player2);                                             // 绘制玩家剩余的墙壁数量
bool IsMouseOnWallButton(int mouseX, int mouseY, int playerId);                                 // 检查鼠标有没有在wall上
void RotationWall(bool &isHorizontal);                                                          // 旋转墙壁
bool IsWallValid(const Wall &wall, const std::vector<Wall> &walls);                             // 检查是否可以放置墙壁
bool IsPathBlocked(int PlayerX, int PlayerY, int GoX, int GoY, const std::vector<Wall> &walls); // 检查路径是否被墙壁阻挡
void ListWalls(const std::vector<Wall> &walls);                                                 // 在terminal显示墙壁信息

// 其他函数
bool CheckVictory(Player player);                                                                                           // 检查获胜
int AnalyzeValidMoves(Player &player, Player &opponent, Vector2 *validMoves, int boardSize, const std::vector<Wall> &walls);// 计算玩家可走选项
void HandlePlayerMove(Player &player, bool &playerSelected, int &currentTurn, int nextTurn, Vector2 validMoves[], int &validMovesCount, int mouseX, int mouseY, float cellSize, float uiHorizon, float uiVertical); // 玩家点击并移动和点击可选路径（黄色小点）

// 主程序
int main()
{
    InitWindow(640, 1000, "Quoridor");
    InitAudioDevice();

    Sound clickSound = LoadSound("assets\\clickSound.wav");
    Sound alert = LoadSound("assets\\game_alert.wav");

    Player player1 = {0, 4, white, 10}; // 玩家1，初始化位置x=0  y = 4  , white color , 持有7个墙壁 （棋盘坐标 = 0⁓9 ， 像素坐标 = （0⁓9 * cellSize） ）
    Player player2 = {8, 4, black, 10}; // 玩家2，初始化位置x=8  y = 4  , black color , 持有7个墙壁 （棋盘坐标 = 0⁓9 ， 像素坐标 = （0⁓9 * cellSize） ）

    bool player1Selected = false; // 玩家没有被selected
    bool player2Selected = false; // 玩家没有被selected

    int currentTurn = 0;    // 绑定与playerID ， 定义为谁的回合

    Vector2 validMoves[6];  // 最多可走选项为6
    int validMovesCount = 0;
    

    std::vector<Wall> walls; // 存储所有墙壁对象  【 wall1 , wall2 , wall3, ...】 
    bool placingWall = false; // 是否正在放置墙壁
    Wall tempWall;            // 预览模式墙壁
    bool isHorizontal = false; // 墙壁方向：默认水平为垂直

    while (!WindowShouldClose())
    {
        int mouseX = GetMouseX();
        int mouseY = GetMouseY();

        // 切换墙壁方向
        RotationWall(isHorizontal);

        // 右键退出预览模式
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        {
            PlaySound(clickSound);
            if (placingWall)
            {
                placingWall = false;     // 退出预览模式
                player1Selected = false; // 取消玩家选择
                player2Selected = false; // 取消玩家选择
                validMovesCount = 0;     // 清除有效路径
            }
        }

        //左键点击墙壁进入预览模式
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            if (IsMouseOnWallButton(mouseX, mouseY, currentTurn)) // 点击墙壁
            {
                PlaySound(clickSound);
                if ((currentTurn == 0 && player1.walls > 0) || (currentTurn == 1 && player2.walls > 0))
                {
                    placingWall = true;      // 进入墙壁放置模式
                    player1Selected = false; // 取消玩家选择
                    player2Selected = false; // 取消玩家选择
                    validMovesCount = 0;     // 清除有效路径
                }
            }
            else if (placingWall) // 已在预览模式
            {

                // 预览墙壁坐标
                int gridX = (mouseX - uiHorizon) / cellSize; 
                int gridY = (mouseY - uiVertical) / cellSize;

                // 检查墙壁的第二个格子是否在棋盘范围内
                bool isSecondCellValid = false;
                if (isHorizontal)
                {
                    isSecondCellValid = (gridX < boardSize - 1); // 水平墙壁的第二个格子是 (x+1, y)
                }
                else
                {
                    isSecondCellValid = (gridY < boardSize - 1); // 垂直墙壁的第二个格子是 (x, y+1)
                }


                if (gridX >= 0 && gridX < boardSize && gridY >= 0 && gridY < boardSize && isSecondCellValid)
                {
                    tempWall = {gridX, gridY, isHorizontal, currentTurn}; // 设置墙壁方向

                    // 检查墙壁是否与已有墙壁重叠
                    bool isOverlapping = !IsWallValid(tempWall, walls);

                    // 预览墙壁正式为实体墙壁，预览模式墙壁和实体墙壁同时存在
                    walls.push_back(tempWall);

                    // 检查路径是否被阻断
                    bool isPathBlockedForPlayer1 = IsPathBlockedForPlayer(player1, walls);
                    bool isPathBlockedForPlayer2 = IsPathBlockedForPlayer(player2, walls);

                    // 移除预览模式墙壁,
                    walls.pop_back();

                    if (isOverlapping)
                    {
                        PlaySound(alert);
                        placementErrorMsg = "Overlaps with another wall!";
                    }
                    else if (isPathBlockedForPlayer1 || isPathBlockedForPlayer2)
                    {
                        PlaySound(alert);
                        placementErrorMsg = "It blocks all paths!";
                    }
                    else
                    {
                        placementErrorMsg = nullptr; // 可以放置，清除提示信息
                    }

                    if (!isOverlapping && !isPathBlockedForPlayer1 && !isPathBlockedForPlayer2)
                    {
                        walls.push_back(tempWall);
                        if (currentTurn == 0)
                            player1.walls--;
                        else
                            player2.walls--;
                        placingWall = false;
                        currentTurn = 1 - currentTurn; // 切换回合
                        validMovesCount = 0;           // 清除有效路径
                        placementErrorMsg = nullptr;
                    }
                }
            }
            else if (IsMouseOnPlayer(mouseX, mouseY, player1) && currentTurn == 0) // 玩家1回合走法
            {
                // 设定 player1 为当前玩家，并初始化可行移动计数
                player1Selected = true;
                player2Selected = false;

                validMovesCount = AnalyzeValidMoves(player1,player2,validMoves,boardSize,walls); // 分析走可选项（player 1）

                
            }
            else if (IsMouseOnPlayer(mouseX, mouseY, player2) && currentTurn == 1) // 玩家2回合走法
            {
                player2Selected = true;
                player1Selected = false;

                validMovesCount = AnalyzeValidMoves(player2,player1,validMoves,boardSize,walls); // 分析可走选项 (player 2)
                
            }
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) // 检测是否点击玩家和点击可走选项
        {
            if (player1Selected)
            {
                HandlePlayerMove(player1, player1Selected, currentTurn, 1, validMoves, validMovesCount ,mouseX, mouseY, cellSize, uiHorizon, uiVertical);
            }
            else if (player2Selected)
            {
                HandlePlayerMove(player2, player2Selected, currentTurn, 0, validMoves , validMovesCount, mouseX, mouseY, cellSize, uiHorizon, uiVertical);
            }
        }
        if (CheckVictory(player1))
        {
            BeginDrawing();
            ClearBackground(white);
            DrawText("Player 1 Wins! ", 200, 280, 30, textcolor);
            EndDrawing();
            PlaySound(alert);
            sleep(3);
            CloseWindow();
            break;
        }
        if (CheckVictory(player2))
        {
            BeginDrawing();
            ClearBackground(white);
            DrawText("Player 2 Wins!", 200, 280, 30, textcolor);
            EndDrawing();
            PlaySound(alert);
            sleep(3);
            CloseWindow();
            break;
        }

        BeginDrawing();

        ClearBackground(background);
        DrawRectangleRounded({0, uiVertical - 50, boardSize * cellSize + uiHorizon + uiHorizon, boardSize * cellSize + 100}, 0.1, 0.0, Board);
        DrawRectangle(50, uiVertical, boardSize * cellSize, boardSize * cellSize, brown);
        DrawText("Quoridor ", 640 / 2 - 120, 60, 50, textcolor);
        DrawText("by lzx", 640 - 30, 780 + uiVertical - 30, 10, textcolor);
        DrawLineEx({20, ((uiVertical - 50) + boardSize * cellSize + 100) + 50}, {540 + uiHorizon + uiHorizon - 20, ((uiVertical - 50) + boardSize * cellSize + 100) + 50}, 3, textcolor);

        // 绘制棋盘
        DrawBoard();
        DrawPosition();

        // 绘制墙壁
        DrawWalls(walls);

        // 绘制玩家
        DrawPlayer(player1);
        DrawPlayer(player2);

        DrawText(currentTurn == 0 ? "Player 1" : "Player 2", 80, boardSize * cellSize + uiVertical + 70, 23, textcolor);

        // 显示可选路径（黄色小点）
        if (player1Selected)
        {
            DrawValidMoves(validMoves, validMovesCount);
        }
        if (player2Selected)
        {
            DrawValidMoves(validMoves, validMovesCount);
        }

        // 绘制墙壁数量
        DrawWallCount(player1, player2);

        // 预览模式：动态绘制墙壁
        if (placingWall)
        {
            int gridX = (mouseX - uiHorizon) / cellSize;
            int gridY = (mouseY - uiVertical) / cellSize;

            // 检查鼠标是否在棋盘范围内
            bool isWithinBoard = gridX >= 0 && gridX < boardSize && gridY >= 0 && gridY < boardSize;

            // 检查墙壁的第二个格子是否在棋盘范围内
            bool isSecondCellValid = false;
            if (isHorizontal)
            {
                isSecondCellValid = (gridX < boardSize - 1); // 水平墙壁的第二个格子是 (x+1, y)
            }
            else
            {
                isSecondCellValid = (gridY < boardSize - 1); // 垂直墙壁的第二个格子是 (x, y+1)
            }

            if (isWithinBoard && isSecondCellValid)
            {
                // 创建一个临时墙壁对象
                Wall previewWall = {gridX, gridY, isHorizontal};

                // 检查墙壁是否与已有墙壁重叠
                bool isOverlapping = !IsWallValid(previewWall, walls);

                // 临时添加到墙壁列表中
                walls.push_back(previewWall);

                // 检查路径是否被阻断
                bool isPathBlockedForPlayer1 = IsPathBlockedForPlayer(player1, walls);
                bool isPathBlockedForPlayer2 = IsPathBlockedForPlayer(player2, walls);

                // 移除临时墙壁
                walls.pop_back();

                // 根据是否重叠或阻断路径设置预览颜色
                Color previewColor = (isOverlapping || isPathBlockedForPlayer1 || isPathBlockedForPlayer2) ? RED : GREEN;

                // 绘制预览墙壁
                if (isHorizontal)
                {
                    DrawRectangle(gridX * cellSize + uiHorizon + 5, gridY * cellSize + uiVertical - 2, cellSize * 2 - 9, 5, previewColor); // 水平墙壁
                }
                else
                {
                    DrawRectangle(gridX * cellSize + uiHorizon - 2, gridY * cellSize + uiVertical + 5, 5, cellSize * 2 - 9, previewColor); // 垂直墙壁
                }
            }
        }
        if (placementErrorMsg) // 绘制错误提醒
        {

            DrawText(placementErrorMsg, 20, boardSize * cellSize + uiVertical + 150, 25, textcolor);
        }

        EndDrawing();
    }
}

// 函数体

bool CheckVictory(Player player) // 检查获胜状态
{
    if (player.color.r == white.r && player.color.g == white.g && player.color.b == white.b && player.x == boardSize - 1)
    {
        return true;
    }
    if (player.color.r == black.r && player.color.g == black.g && player.color.b == black.b && player.x == 0)
    {
        return true;
    }
    return false;
} // 用rgb颜色判断红方还是蓝方赢

void DrawBoard() // 绘制棋盘
{
    for (int row = 0; row < boardSize; row++)
    {
        for (int col = 0; col < boardSize; col++)
        {
            // 调整 X 坐标，增加 uiMargin
            Rectangle cell = {
                (float)(uiHorizon + col * cellSize),  // X 坐标增加 uiMargin
                (float)(row * cellSize + uiVertical), // Y 坐标不变
                (float)cellSize,
                (float)cellSize};
            DrawRectangleLinesEx(cell, 4, line);
        }
    }
}

void DrawPosition() // 绘制棋盘的坐标
{
    DrawText("9", 23, uiVertical + 18, 18, textcolor);
    DrawText("8", 23, uiVertical + 18 + 60 * 1, 18, textcolor);
    DrawText("7", 23, uiVertical + 18 + 60 * 2, 18, textcolor);
    DrawText("6", 23, uiVertical + 18 + 60 * 3, 18, textcolor);
    DrawText("5", 23, uiVertical + 18 + 60 * 4, 18, textcolor);
    DrawText("4", 23, uiVertical + 18 + 60 * 5, 18, textcolor);
    DrawText("3", 23, uiVertical + 18 + 60 * 6, 18, textcolor);
    DrawText("2", 23, uiVertical + 18 + 60 * 7, 18, textcolor);
    DrawText("1", 25, uiVertical + 18 + 60 * 8, 20, textcolor);

    DrawText("a", 45 + cellSize / 2, boardSize * cellSize + uiVertical + 10, 18, textcolor);
    DrawText("b", 45 + cellSize / 2 + 60 * 1, boardSize * cellSize + uiVertical + 10, 18, textcolor);
    DrawText("c", 45 + cellSize / 2 + 60 * 2, boardSize * cellSize + uiVertical + 10, 18, textcolor);
    DrawText("d", 45 + cellSize / 2 + 60 * 3, boardSize * cellSize + uiVertical + 10, 18, textcolor);
    DrawText("e", 45 + cellSize / 2 + 60 * 4, boardSize * cellSize + uiVertical + 10, 18, textcolor);
    DrawText("f", 45 + cellSize / 2 + 60 * 5, boardSize * cellSize + uiVertical + 10, 18, textcolor);
    DrawText("g", 45 + cellSize / 2 + 60 * 6, boardSize * cellSize + uiVertical + 10, 18, textcolor);
    DrawText("h", 45 + cellSize / 2 + 60 * 7, boardSize * cellSize + uiVertical + 10, 18, textcolor);
    DrawText("i", 45 + cellSize / 2 + 60 * 8, boardSize * cellSize + uiVertical + 10, 18, textcolor);
}

void DrawPlayer(Player player) // 绘制玩家
{
    DrawCircle(player.x * cellSize + cellSize / 2 + uiHorizon, player.y * cellSize + cellSize / 2 + uiVertical, cellSize / 4, player.color);
}

void DrawWalls(const std::vector<Wall> &walls) // 绘制墙壁
{
    for (const auto &wall : walls)
    {
        Color wallColor = (wall.playerid == 0) ? white : black; // 根据 playerid 选择颜色
        if (wall.horizontal)
        {
            // 水平墙壁
            DrawRectangle(wall.x * cellSize + uiHorizon + 5, wall.y * cellSize + uiVertical - 5, cellSize * 2 - 9, 10, wallColor);
        }
        else
        {
            // 垂直墙壁
            DrawRectangle(wall.x * cellSize + uiHorizon - 5, wall.y * cellSize + uiVertical + 5, 10, cellSize * 2 - 9, wallColor);
        }
    }
}

bool IsMouseOnPlayer(int mouseX, int mouseY, Player player) // 检查是否点击到玩家角色
{
    // playerCenterX 获取了格子中心点
    int playerCenterX = player.x * cellSize + cellSize / 2 + uiHorizon;
    int playerCenterY = player.y * cellSize + cellSize / 2 + uiVertical;

    return (mouseX >= playerCenterX - cellSize / 4 && mouseX <= playerCenterX + cellSize / 4) && (mouseY >= playerCenterY - cellSize / 4 && mouseY <= playerCenterY + cellSize / 4); // return true or false
    //  想象一个四方形蛋糕被切成4片，其中两片靠近中间才在鼠标范围内
}

bool IsMouseOnWallButton(int mouseX, int mouseY, int playerId) // 检查鼠标有没有在wall上
{
    if (playerId == 0)
    { // 玩家1
        return mouseX >= (540 + uiHorizon + uiHorizon) / 2 - 70 && mouseX <= (540 + uiHorizon + uiHorizon) / 2 + 20 && mouseY >= boardSize * cellSize + uiVertical + 70 && mouseY <= boardSize * cellSize + uiVertical + 70 + 23;
    }
    else if (playerId == 1)
    { // 玩家2
        return mouseX >= (540 + uiHorizon + uiHorizon) / 2 + 100 && mouseX <= (540 + uiHorizon + uiHorizon) / 2 + 100 + 70 && mouseY >= boardSize * cellSize + uiVertical + 70 && mouseY <= boardSize * cellSize + uiVertical + 70 + 23;
    }
    return false;
}

void DrawWallCount(Player player1, Player player2) // 绘制墙壁数量UI
{
    DrawText(TextFormat("WHITE   %d", player1.walls), (540 + uiHorizon + uiHorizon) / 2 - 70, boardSize * cellSize + uiVertical + 70, 23, textcolor);

    DrawText(TextFormat("BLACK   %d", player2.walls), (540 + uiHorizon + uiHorizon) / 2 + 100, boardSize * cellSize + uiVertical + 70, 23, textcolor);
}

bool IsWallValid(const Wall &wall, const std::vector<Wall> &walls) // 检查是否可以放置墙壁
{
    // 检查墙壁的第二个格子是否超出棋盘范围
    if (wall.horizontal)
    {
        if (wall.x >= boardSize - 1 || wall.y >= boardSize) // 水平墙壁的第二个格子是 (x+1, y)
            return false;
    }
    else
    {
        if (wall.x >= boardSize || wall.y >= boardSize - 1) // 垂直墙壁的第二个格子是 (x, y+1)
            return false;
    }

    // 检查墙壁的两个格子是否与已有墙壁的任意一个格子重叠
    for (const auto &existingWall : walls)
    {
        if (existingWall.horizontal == wall.horizontal)
        {
            // 同方向墙壁：检查是否完全重叠
            if (existingWall.horizontal)
            {
                // 已有墙壁是水平方向
                if ((wall.x == existingWall.x && wall.y == existingWall.y) ||     // 检查第一个格子
                    (wall.x + 1 == existingWall.x && wall.y == existingWall.y) || // 检查第二个格子
                    (wall.x == existingWall.x + 1 && wall.y == existingWall.y))   // 检查与已有墙壁的重叠
                {
                    return false; // 重叠
                }
            }
            else
            {
                // 已有墙壁是垂直方向
                if ((wall.x == existingWall.x && wall.y == existingWall.y) ||     // 检查第一个格子
                    (wall.x == existingWall.x && wall.y + 1 == existingWall.y) || // 检查第二个格子
                    (wall.x == existingWall.x && wall.y == existingWall.y + 1))   // 检查与已有墙壁的重叠
                {
                    return false; // 重叠
                }
            }
        }
        else
        {
            // 不同方向墙壁：允许交叉，不检查重叠
            continue;
        }
    }
    return true; // 没有重叠
}

void RotationWall(bool &isHorizontal) // 旋转墙壁
{
    // Q 和 E 具备寻找墙壁功能
    if (IsKeyPressed(KEY_Q))
    {
        if (isHorizontal == true)
        {
            isHorizontal = false;
        }
        else if (isHorizontal == false)
        {
            isHorizontal = true;
        }
    }
    if (IsKeyPressed(KEY_E))
    {
        if (isHorizontal == false)
        {
            isHorizontal = true;
        }
        else if (isHorizontal == true)
        {
            isHorizontal = false;
        }
    }
}

bool IsPathBlocked(int PlayerX, int PlayerY, int GoX, int GoY, const std::vector<Wall> &walls) // 检查路径是否被墙壁阻挡
{
    if (PlayerY == GoY)
    {
        // 左右移动
        int minX = (PlayerX < GoX) ? PlayerX : GoX; // 找到较小的 x 坐标
        for (const auto &wall : walls)
        {
            // 检查是否有垂直墙壁位于当前单元格和目标单元格之间的垂直线上
            if (!wall.horizontal && wall.x - 1 == minX && wall.y == PlayerY)
            {
                return true; // 垂直墙壁阻挡水平移动
            }
            else if (!wall.horizontal && wall.x - 1 == minX && wall.y + 1 == PlayerY)
            {
                return true; // 垂直墙壁阻挡水平移动
            }
        }
    }
    else if (PlayerX == GoX)
    {
        // 垂直移动
        int minY = (PlayerY < GoY) ? PlayerY : GoY; // 找到较小的 y 坐标
        for (const auto &wall : walls)
        {
            // 检查是否有水平墙壁位于当前单元格和目标单元格之间的水平线上
            if (wall.horizontal && wall.x == PlayerX && wall.y == minY + 1)
            {
                return true; // 水平墙壁阻挡垂直移动
            }
            else if (wall.horizontal && wall.x + 1 == PlayerX && wall.y == minY + 1)
            {
                return true; // 水平墙壁阻挡垂直移动
            }
        }
    }
    return false; // 路径未被阻挡
}

void DrawValidMoves(Vector2 validMoves[], int validMovesCount) // 绘制可选路径（黄色小点）
{
    for (int i = 0; i < validMovesCount; i++)
    {
        DrawCircle(validMoves[i].x * cellSize + cellSize / 2 + uiHorizon, validMoves[i].y * cellSize + cellSize / 2 + uiVertical, 5, YELLOW); // Y坐标加uiVertical
    }
}

bool IsPathBlockedForPlayer(Player player, const std::vector<Wall> &walls) // 检查玩家放置墙壁是否阻挡可选路径
{
    // 玩家的目标端
    int targetX = (player.color.r == white.r && player.color.g == white.g && player.color.b == white.b) ? boardSize - 1 : 0; // 判断终点在0还是8

    // BFS 队列
    std::queue<std::pair<int, int>> q;
    q.push({player.x, player.y});

    // 访问标记
    std::unordered_set<int> visited;
    visited.insert(player.x * boardSize + player.y);

    // 方向数组：上、下、左、右
    int dx[] = {0, 0, -1, 1};
    int dy[] = {-1, 1, 0, 0};

    while (!q.empty())
    {
        auto current = q.front();
        q.pop();
        int x = current.first;
        int y = current.second;

        // 到达目标端
        if (x == targetX)
            return false; // 路径未被阻断

        // 遍历四个方向
        for (int i = 0; i < 4; i++)
        {
            int nx = x + dx[i];
            int ny = y + dy[i];

            // 检查是否在棋盘范围内
            if (nx >= 0 && nx < boardSize && ny >= 0 && ny < boardSize)
            {
                // 检查路径是否被墙壁阻挡
                bool isBlocked = false;
                if (dx[i] == -1 && IsPathBlocked(x, y, nx, ny, walls)) // 左
                    isBlocked = true;
                else if (dx[i] == 1 && IsPathBlocked(x, y, nx, ny, walls)) // 右
                    isBlocked = true;
                else if (dy[i] == -1 && IsPathBlocked(x, y, nx, ny, walls)) // 上
                    isBlocked = true;
                else if (dy[i] == 1 && IsPathBlocked(x, y, nx, ny, walls)) // 下
                    isBlocked = true;

                if (!isBlocked)
                {
                    int key = nx * boardSize + ny;
                    if (visited.find(key) == visited.end())
                    {
                        visited.insert(key);
                        q.push({nx, ny});
                    }
                }
            }
        }
    }

    return true; // 路径被阻断
}

void ListWalls(const std::vector<Wall> &walls) // 在terminal显示墙壁信息
{
    for (size_t i = 0; i < walls.size(); i++)
    {
        const Wall &wall = walls[i];
        const char *direction = wall.horizontal ? "Horizontal" : "Vertical";
        printf("Wall %zu: x = %d, y = %d, direction = %s\n", i + 1, wall.x, wall.y, direction);
    }
}

int AnalyzeValidMoves(Player &player, Player &opponent, Vector2 *validMoves, int boardSize, const std::vector<Wall> &walls) // 分析玩家可走选项
{
    printf("\n\n Another Round \n\n");

    int count = 0; // 有效移动的计数器

    // 检查上下左右的基本移动
    // 检查向左移动是否有效
    if (player.x > 0 && !(player.x - 1 == opponent.x && player.y == opponent.y) &&
        !IsPathBlocked(player.x, player.y, player.x - 1, player.y, walls))
    {
        validMoves[count++] = {(float)(player.x - 1), (float)(player.y)}; // 添加向左的有效移动
    }
    // 检查向右移动是否有效
    if (player.x < boardSize - 1 && !(player.x + 1 == opponent.x && player.y == opponent.y) &&
        !IsPathBlocked(player.x, player.y, player.x + 1, player.y, walls))
    {
        validMoves[count++] = {(float)(player.x + 1), (float)(player.y)}; // 添加向右的有效移动
    }
    // 检查向上移动是否有效
    if (player.y > 0 && !(player.x == opponent.x && player.y - 1 == opponent.y) &&
        !IsPathBlocked(player.x, player.y, player.x, player.y - 1, walls))
    {
        validMoves[count++] = {(float)(player.x), (float)(player.y - 1)}; // 添加向上的有效移动
    }
    // 检查向下移动是否有效
    if (player.y < boardSize - 1 && !(player.x == opponent.x && player.y + 1 == opponent.y) &&
        !IsPathBlocked(player.x, player.y, player.x, player.y + 1, walls))
    {
        validMoves[count++] = {(float)(player.x), (float)(player.y + 1)}; // 添加向下的有效移动
    }

    // 处理跳跃逻辑 - 当对手在相邻位置时的特殊移动规则

    // 处理对手在左侧的跳跃情况
    if (player.x > 1 && (player.x - 1 == opponent.x && player.y == opponent.y) &&
        !IsPathBlocked(player.x, player.y, player.x - 1, player.y, walls))
    {
        // 尝试直接向左跳过对手
        if (!IsPathBlocked(player.x - 1, player.y, player.x - 2, player.y, walls))
        {
            validMoves[count++] = {(float)(player.x - 2), (float)(player.y)}; // 跳过对手到左侧两格
        }
        else
        {
            // 如果不能直接跳过，尝试对角线跳跃（左上或左下）
            if (player.y > 0 && !IsPathBlocked(player.x - 1, player.y, player.x - 1, player.y - 1, walls))
            {
                validMoves[count++] = {(float)(player.x - 1), (float)(player.y - 1)}; // 添加左上对角线跳跃
            }
            if (player.y < boardSize - 1 && !IsPathBlocked(player.x - 1, player.y, player.x - 1, player.y + 1, walls))
            {
                validMoves[count++] = {(float)(player.x - 1), (float)(player.y + 1)}; // 添加左下对角线跳跃
            }
        }
    }

    // 处理对手在右侧的跳跃情况
    if (player.x < boardSize - 2 && (player.x + 1 == opponent.x && player.y == opponent.y) &&
        !IsPathBlocked(player.x, player.y, player.x + 1, player.y, walls))
    {
        // 尝试直接向右跳过对手
        if (!IsPathBlocked(player.x + 1, player.y, player.x + 2, player.y, walls))
        {
            validMoves[count++] = {(float)(player.x + 2), (float)(player.y)}; // 跳过对手到右侧两格
        }
        else
        {
            // 如果不能直接跳过，尝试对角线跳跃（右上或右下）
            if (player.y > 0 && !IsPathBlocked(player.x + 1, player.y, player.x + 1, player.y - 1, walls))
            {
                validMoves[count++] = {(float)(player.x + 1), (float)(player.y - 1)}; // 添加右上对角线跳跃
            }
            if (player.y < boardSize - 1 && !IsPathBlocked(player.x + 1, player.y, player.x + 1, player.y + 1, walls))
            {
                validMoves[count++] = {(float)(player.x + 1), (float)(player.y + 1)}; // 添加右下对角线跳跃
            }
        }
    }

    // 处理对手在上方的跳跃情况
    if (player.y > 1 && (player.x == opponent.x && player.y - 1 == opponent.y) &&
        !IsPathBlocked(player.x, player.y, player.x, player.y - 1, walls))
    {
        // 尝试直接向上跳过对手
        if (!IsPathBlocked(player.x, player.y - 1, player.x, player.y - 2, walls))
        {
            validMoves[count++] = {(float)(player.x), (float)(player.y - 2)}; // 跳过对手到上方两格
        }
        else
        {
            // 如果不能直接跳过，尝试对角线跳跃（左上或右上）
            if (player.x > 0 && !IsPathBlocked(player.x, player.y - 1, player.x - 1, player.y - 1, walls))
            {
                validMoves[count++] = {(float)(player.x - 1), (float)(player.y - 1)}; // 添加左上对角线跳跃
            }
            if (player.x < boardSize - 1 && !IsPathBlocked(player.x, player.y - 1, player.x + 1, player.y - 1, walls))
            {
                validMoves[count++] = {(float)(player.x + 1), (float)(player.y - 1)}; // 添加右上对角线跳跃
            }
        }
    }

    // 处理对手在下方的跳跃情况
    if (player.y < boardSize - 2 && (player.x == opponent.x && player.y + 1 == opponent.y) &&
        !IsPathBlocked(player.x, player.y, player.x, player.y + 1, walls))
    {
        // 尝试直接向下跳过对手
        if (!IsPathBlocked(player.x, player.y + 1, player.x, player.y + 2, walls))
        {
            validMoves[count++] = {(float)(player.x), (float)(player.y + 2)}; // 跳过对手到下方两格
        }
        else
        {
            // 如果不能直接跳过，尝试对角线跳跃（左下或右下）
            if (player.x > 0 && !IsPathBlocked(player.x, player.y + 1, player.x - 1, player.y + 1, walls))
            {
                validMoves[count++] = {(float)(player.x - 1), (float)(player.y + 1)}; // 添加左下对角线跳跃
            }
            if (player.x < boardSize - 1 && !IsPathBlocked(player.x, player.y + 1, player.x + 1, player.y + 1, walls))
            {
                validMoves[count++] = {(float)(player.x + 1), (float)(player.y + 1)}; // 添加右下对角线跳跃
            }
        }
    }

    ListWalls(walls);

    return count; // 返回有效移动的总数
}

void HandlePlayerMove(Player &player, bool &playerSelected, int &currentTurn, int nextTurn, Vector2 validMoves[], int &validMovesCount, int mouseX, int mouseY, float cellSize, float uiHorizon, float uiVertical) // 玩家点击并移动和点击玩家可选路径（黄色小点）
{
    for (int i = 0; i < validMovesCount; i++)
    { // 遍历所有有效移动位置
        // 检查鼠标点击的坐标是否落在有效移动的位置上
        if (mouseX >= validMoves[i].x * cellSize + uiHorizon &&
            mouseX <= (validMoves[i].x + 1) * cellSize + uiHorizon &&
            mouseY >= validMoves[i].y * cellSize + uiVertical &&
            mouseY <= (validMoves[i].y + 1) * cellSize + uiVertical)
        {

            // 如果点击位置在有效移动范围内，更新玩家位置
            player.x = validMoves[i].x;
            player.y = validMoves[i].y;

            // 取消当前玩家选中状态
            playerSelected = false;

            // 切换当前回合
            currentTurn = nextTurn;

            // 清空有效移动列表，防止误操作
            validMovesCount = 0;
            break; // 结束循环，防止多次更新
        }
    }
}
