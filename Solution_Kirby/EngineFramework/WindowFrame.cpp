#include "pch.h"
#include "Scene.h"

WindowFrame* WindowFrame::m_Pthis = nullptr;

void WindowFrame::Create(HINSTANCE hInstance)
{
	if (!m_Pthis)
	{
		m_Pthis = new WindowFrame();
		m_Pthis->m_Instance = hInstance;
	}
}

WindowFrame* WindowFrame::GetInstance()
{
	return m_Pthis;
}

void WindowFrame::Destroy()
{
	if (m_Pthis)
	{
		if (m_Pthis->m_scene)
		{
			delete m_Pthis->m_scene;
		}
		delete m_Pthis;
		m_Pthis = nullptr;
	}
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WindowFrame::WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	static HDC hdc;
	static PAINTSTRUCT ps;

	if (ImGui_ImplWin32_WndProcHandler(hWnd, iMessage, wParam, lParam))
	{
		return true; // 메시지가 ImGui에 의해 처리되었음을 나타냄
	}

	switch (iMessage)
	{
	case WM_CREATE:
		return 0;
	case WM_LBUTTONDOWN:
		Mouse::GetInstance()->SetLeftBtn(true);
		Mouse::GetInstance()->SetPos(LOWORD(lParam), HIWORD(lParam));
		ObjectManager::GetInstance()->OnLBtnDown();
		return 0;
	case WM_LBUTTONUP:
		Mouse::GetInstance()->SetLeftBtn(false);
		Mouse::GetInstance()->SetPos(LOWORD(lParam), HIWORD(lParam));
		ObjectManager::GetInstance()->OnLBtnUp();
		return 0;
	case WM_RBUTTONDOWN:
		SetFocus(hWnd);
		Mouse::GetInstance()->SetRightBtn(true);
		Mouse::GetInstance()->SetPos(LOWORD(lParam), HIWORD(lParam));
		ObjectManager::GetInstance()->OnRBtnDown();
		return 0;
	case WM_RBUTTONUP:
		Mouse::GetInstance()->SetRightBtn(false);
		Mouse::GetInstance()->SetPos(LOWORD(lParam), HIWORD(lParam));
		ObjectManager::GetInstance()->OnRBtnUp();
		return 0;
	case WM_MOUSEMOVE:
		Mouse::GetInstance()->SetPos(LOWORD(lParam), HIWORD(lParam));
		return 0;
	case WM_KILLFOCUS:
		return 0;
	case WM_SETFOCUS:
		return 0;
	case WM_SIZE:
		return 0;
	case WM_PAINT:
		ValidateRect(hWnd, NULL);
		return 0;
	case WM_DESTROY:
		m_Pthis->m_scene->Release();
		PostQuitMessage(0);
		return 0;
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}

void WindowFrame::SetScene(Scene* scene)
{
	if (m_scene)
	{
		m_scene->Release();
		ObjectManager::GetInstance()->Clear();
		TextureManager::GetInstance()->ReleaseAllTextures();
		delete m_scene;
	}
	m_scene = scene;
	m_scene->Init();
	m_scene->Start();
}

void WindowFrame::Initialize(RenderType type)
{
	BuildWindow();
	if(type == RenderType::Game)
		MoveWindow(m_Pthis->m_hWnd, 100, 100, DRAWWINDOWW, DRAWWINDOWH, TRUE);
	SetFocus(m_Pthis->m_hWnd);
}

void WindowFrame::BuildWindow()
{
	WNDCLASSEX WndClass = 
	{
	sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L,
	GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
	"MyWindow", NULL
	};

	RegisterClassEx(&WndClass);

	m_Pthis->m_hWnd =
		CreateWindow(WndClass.lpszClassName, "최원준",
			WS_OVERLAPPED | 
			WS_CAPTION | 
			WS_SYSMENU | 
			WS_MINIMIZEBOX | 
			WS_CLIPSIBLINGS | 
			WS_CLIPCHILDREN |
			WS_SIZEBOX,
			0, 0, MAXWINDOWW, MAXWINDOWH,
			NULL, (HMENU)NULL, m_Pthis->m_Instance, NULL);

	ShowWindow(m_Pthis->m_hWnd, SW_SHOWDEFAULT);
	UpdateWindow(m_Pthis->m_hWnd);

}

void WindowFrame::Run(const MSG* Message)
{
	TranslateMessage(Message);
	DispatchMessage(Message);
}
