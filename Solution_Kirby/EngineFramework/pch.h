#pragma once
#define FBXSDK_SHARED

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
#include <d3dx9.h>
#include <directxmath.h>

#include "../Box2D/box2d/include/box2d/box2d.h"
#include "../Imgui/imgui/imgui.h"
#include "../Imgui/source/imgui_impl_dx9.h"
#include "../Imgui/source/imgui_impl_win32.h"

#include "../fbxsdk/fbxsdk.h"

#include "SharedPointer.h"

#include "ObjectPool.h"
#include "FbxTool.h"
#include "DebugRender.h"
#include "AnimationManager.h"
#include "ObjectManager.h"
#include "TextureManager.h"
//#include "CollisionManager.h"
#include "MainFrame.h"
#include "WindowFrame.h"
#include "Mouse.h"
#include "Camera.h"
#include "Buffer.h"


//#include "game.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define TRANSCOLORWIN (RGB((255),(0),(255)))
#define TRANSCOLORDX (D3DCOLOR_XRGB((255),(0),(255)))

#define COLSIZEOFFSET 1.0f
#define MAXWINDOWW 1600
#define MAXWINDOWH 1000
#define DRAWWINDOWW 1440
#define DRAWWINDOWH 900
#define BUFFERBITW DRAWWINDOWW
#define BUFFERBITH DRAWWINDOWH
#define TARGETFPS 120

#ifdef _DEBUG
#define DEBUGMODE true
#else 
#define DEBUGMODE false
#endif
#define DEBUGCOLOR1 (RGB((0),(0),(255)))
#define DEBUGCOLOR2 (RGB((255),(0),(0)))

#define DEBUGCOLORDX1 (D3DCOLOR_XRGB((0),(0),(255)))
#define DEBUGCOLORDX2 (D3DCOLOR_XRGB((255),(0),(0)))

using namespace DirectX;
using namespace std;

struct CUSTOMVERTEX
{
    FLOAT x, y, z;         // The transformed position for the vertex
    DWORD color;        // The vertex color
    FLOAT tu, tv;
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)

struct DEBUGVERTEX
{
    FLOAT x, y, z;         // The transformed position for the vertex
    DWORD color;        // The vertex color
};
#define D3DFVF_DEBUGVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)
// ���׸��� ����ü
struct Material
{
    std::vector<std::string> texturePaths;
    std::vector<IDirect3DTexture9*> textures;
    D3DCOLOR diffuseColor;  // ������ Ȯ�� ����
};

struct SubMesh {
    unsigned int startIndex; // ���� �ε���
    unsigned int indexCount; // �ε��� ��
    unsigned int vertexCount; // ���� ��
    unsigned int vertexStart;
    D3DCOLOR diffuseColor; // ����
    std::vector<IDirect3DTexture9*> textures; // �ؽ�ó ���

    FLOAT uvScaleU;    // UV ������ U
    FLOAT uvScaleV;    // UV ������ V
    FLOAT uvOffsetU;   // UV ������ U
    FLOAT uvOffsetV;   // UV ������ V
};

//��
struct Model
{
    std::vector<CUSTOMVERTEX> vertices;
    std::vector<unsigned int> indices;
    std::vector<SubMesh> subMeshes;  // ���� �޽� ���
    IDirect3DVertexBuffer9* vertexBuffer = nullptr;
    IDirect3DIndexBuffer9* indexBuffer = nullptr;
    int indexCount = 0;
    int vertexCount = 0;
    // ������: ���� �ʱ�ȭ
    Model() = default;

    // �Ҹ���: ���� ����
    ~Model() {
        if (vertexBuffer) {
            vertexBuffer->Release();
            vertexBuffer = nullptr;
        }
        if (indexBuffer) {
            indexBuffer->Release();
            indexBuffer = nullptr;
        }
    }
};

D3DXVECTOR3 WorldToScreen(LPDIRECT3DDEVICE9 pd3dDevice, const D3DXVECTOR3& worldPos);
D3DXVECTOR3 ScreenToWorld(LPDIRECT3DDEVICE9 pd3dDevice, const D3DXVECTOR3& screenPos);

void DrawTextInRect(HDC hdc, const std::wstring& text, const RECT& rect);

wstring ConvertToWideString(const std::string& narrowStr);
string ConvertToString(const std::wstring& wstr);

void FillRectWithColor(HDC hdc, const RECT& rect, COLORREF color);


