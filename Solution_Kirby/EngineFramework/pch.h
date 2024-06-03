#pragma once
#pragma comment(linker,"/entry:WinMainCRTStartup /subsystem:console")
#pragma comment (lib, "msimg32.lib")
#include <cmath>
#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <algorithm>
#include <typeinfo>
#include <vector>
#include <list>
#include <set>
#include <string>
#include <math.h>
#include <mmsystem.h>
#include <time.h>
#include <timeapi.h>
#include <fstream>
#include <locale>
#include <codecvt>
#include <sstream>
#include <functional>
#include <stack>

#include "AnimationManager.h"
#include "ObjectManager.h"
#include "CollisionManager.h"
#include "MainFrame.h"
#include "WindowFrame.h"
#include "Mouse.h"
#include "Camera.h"
#include "Buffer.h"


//#include "game.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define TRANSCOLOR (RGB((255),(0),(255)))
#define COLSIZEOFFSET 1.0f
#define MAXWINDOWW 2560
#define MAXWINDOWH 1440
#define BUFFERBITW MAXWINDOWW
#define BUFFERBITH MAXWINDOWH
#define TARGETFPS 120

#ifdef _DEBUG
#define DEBUGMODE true
#else 
#define DEBUGMODE false
#endif
#define DEBUGCOLOR1 (RGB((0),(0),(255)))
#define DEBUGCOLOR2 (RGB((255),(0),(0)))


using namespace std;


void DrawTextInRect(HDC hdc, const std::wstring& text, const RECT& rect);

wstring ConvertToWideString(const std::string& narrowStr);

void FillRectWithColor(HDC hdc, const RECT& rect, COLORREF color);