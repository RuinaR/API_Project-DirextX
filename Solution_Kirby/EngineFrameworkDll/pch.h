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
#include <map>
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
#include "Resource/ResourceManager.h"
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
#define TARGETFPS 120

#define LOGICAL_RENDER_WIDTH 1440
#define LOGICAL_RENDER_HEIGHT 900

// 현재 런타임은 기본적으로 고정 client 크기로 시작하지만,
// 창 크기와 논리 렌더 해상도는 서로 다른 책임으로 분리해 관리한다.
#define DEFAULT_WINDOW_CLIENT_WIDTH LOGICAL_RENDER_WIDTH
#define DEFAULT_WINDOW_CLIENT_HEIGHT LOGICAL_RENDER_HEIGHT

// 카메라 기본 줌은 논리 렌더 해상도와 분리해서,
// 해상도를 바꿔도 월드 스케일이 암묵적으로 바뀌지 않게 한다.
#define DEFAULT_CAMERA_ORTHOGRAPHIC_SIZE 900.0f

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

inline std::string WideToSystemString(const wchar_t* value)
{
    if (!value)
    {
        return std::string();
    }

    const int size = WideCharToMultiByte(CP_ACP, 0, value, -1, nullptr, 0, nullptr, nullptr);
    if (size <= 0)
    {
        return std::string();
    }

    std::string result(static_cast<size_t>(size - 1), '\0');
    WideCharToMultiByte(CP_ACP, 0, value, -1, &result[0], size, nullptr, nullptr);
    return result;
}

inline std::string WideToAnsi(const wchar_t* value)
{
    return WideToSystemString(value);
}

#define USES_CONVERSION
#define W2A(value) WideToAnsi(value).c_str()
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
// 메테리얼 구조체
struct Material
{
    std::vector<std::string> texturePaths;
    std::vector<IDirect3DTexture9*> textures;
    D3DCOLOR diffuseColor;  // 재질의 확산 색상
};

struct SubMesh {
    unsigned int startIndex; // 시작 인덱스
    unsigned int indexCount; // 인덱스 수
    unsigned int vertexCount; // 정점 수
    unsigned int vertexStart;
    unsigned int materialIndex;
    D3DCOLOR diffuseColor; // 색상
    std::vector<IDirect3DTexture9*> textures; // 텍스처 목록

    FLOAT uvScaleU;    // UV 스케일 U
    FLOAT uvScaleV;    // UV 스케일 V
    FLOAT uvOffsetU;   // UV 오프셋 U
    FLOAT uvOffsetV;   // UV 오프셋 V
};

//모델
struct Model
{
    std::vector<CUSTOMVERTEX> vertices;
    std::vector<unsigned int> indices;
    std::vector<SubMesh> subMeshes;  // 서브 메시 목록
    IDirect3DVertexBuffer9* vertexBuffer = nullptr;
    IDirect3DIndexBuffer9* indexBuffer = nullptr;
    int indexCount = 0;
    int vertexCount = 0;
    // 생성자: 버퍼 초기화
    Model() = default;

    // 소멸자: 버퍼 해제
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

ENGINEFRAMEWORK_API D3DXVECTOR3 WorldToScreen(LPDIRECT3DDEVICE9 pd3dDevice, const D3DXVECTOR3* worldPos);
ENGINEFRAMEWORK_API D3DXVECTOR3 ScreenToWorld(LPDIRECT3DDEVICE9 pd3dDevice, const D3DXVECTOR3* screenPos);

ENGINEFRAMEWORK_API void DrawTextInRect(HDC hdc, const std::wstring& text, const RECT* rect);

ENGINEFRAMEWORK_API wstring ConvertToWideString(const std::string& narrowStr);
ENGINEFRAMEWORK_API string ConvertToString(const std::wstring& wstr);

ENGINEFRAMEWORK_API void FillRectWithColor(HDC hdc, const RECT* rect, COLORREF color);


