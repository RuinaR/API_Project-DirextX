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

LRESULT WindowFrame::WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	static HDC hdc;
	static PAINTSTRUCT ps;
	switch (iMessage)
	{
	case WM_CREATE:
		return 0;
	case WM_LBUTTONDOWN:
		SetFocus(hWnd);
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
		m_Pthis->m_isFocus = false;
		return 0;
	case WM_SETFOCUS:
		MainFrame::GetInstance()->Timer().tick();
		MainFrame::GetInstance()->Timer().resetTotalDeltaTime();
		m_Pthis->m_isFocus = true;
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
		delete m_scene;
	}
	m_scene = scene;
	m_scene->Init();
	m_scene->Start();
}

void WindowFrame::Initialize()
{
	BuildWindow();
	MoveWindow(m_Pthis->m_hWnd, 100, 100, 1440, 900, TRUE);
	SetFocus(m_Pthis->m_hWnd);
}

void WindowFrame::BuildWindow()
{
	WNDCLASSEX WndClass = 
	{
	sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L,
	GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
	L"MyWindow", NULL
	};

	RegisterClassEx(&WndClass);

	m_Pthis->m_hWnd =
		CreateWindow(WndClass.lpszClassName, L"ÃÖ¿øÁØ",
			WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
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
