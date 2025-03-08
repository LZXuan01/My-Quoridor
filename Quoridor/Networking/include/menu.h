#ifndef MENU_H
#define MENU_H

#include "raylib.h"

extern Font myFont ;  // 声明 myFont 变量
extern Sound clickSound ;
extern Sound alertSound ;

void InitMenu();   // 初始化菜单界面
void DrawMenu();   // 绘制菜单界面
void DrawRoom();
int GameStart();
int ClickButton(); // 处理按钮点击事件
void UnloadMenu(); // 释放资源
void ScreenFadeIn();// 淡入动画

#endif

