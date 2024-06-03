#include "pch.h"
#include "Scene.h"

WindowFrame* WindowFrame::m_Pthis = nullptr;

void WindowFrame::Create(HINSTANCE hInstance)
{
	if (!m_Pthis)
	{
		m_Pthis = new WindowFrame();
		m_Pthis->m_Instance = hInstance;
		m_Pthis->m_buffer = new Buffer();
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
		delete m_Pthis->m_buffer;
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
		hdc = BeginPaint(hWnd, &ps);
		m_Pthis->m_buffer->CopyBitmap(hdc);
		m_Pthis->m_buffer->DrawBG();
		EndPaint(hWnd, &ps);
		return 0;
	case WM_DESTROY:
		m_Pthis->m_scene->Release();
		m_Pthis->m_buffer->Release();
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
	m_Pthis->m_buffer->Init(m_Pthis->m_hWnd);
	MoveWindow(m_Pthis->m_hWnd, 100, 100, 1440, 900, TRUE);
	SetFocus(m_Pthis->m_hWnd);
}

void WindowFrame::BuildWindow()
{
	WNDCLASS WndClass;

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = m_Pthis->m_Instance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = TEXT("ÃÖ¿øÁØ");
	WndClass.lpszMenuName = NULL;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	m_Pthis->m_hWnd =
		CreateWindow(WndClass.lpszClassName, WndClass.lpszClassName,
			WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
			0, 0, MAXWINDOWW, MAXWINDOWH,
			NULL, (HMENU)NULL, m_Pthis->m_Instance, NULL);

	ShowWindow(m_Pthis->m_hWnd, SW_SHOW);
}

void WindowFrame::Run(const MSG* Message)
{
	TranslateMessage(Message);
	DispatchMessage(Message);
}
