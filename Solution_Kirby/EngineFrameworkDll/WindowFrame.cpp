#include "pch.h"
#include "Scene.h"
#include "SceneDataManager.h"
#include "Resource/ResourceManager.h"

WindowFrame* WindowFrame::m_Pthis = nullptr;

namespace
{
	constexpr DWORD kResizableWindowStyle =
		WS_OVERLAPPEDWINDOW |
		WS_CLIPSIBLINGS |
		WS_CLIPCHILDREN;

	RECT BuildStartupWindowRect()
	{
		RECT rect = { 0, 0, DEFAULT_WINDOW_CLIENT_WIDTH, DEFAULT_WINDOW_CLIENT_HEIGHT };
		// 시작 client 크기만 기본값으로 맞추고, 실행 중에는 창 리사이즈를 허용한다.
		AdjustWindowRect(&rect, kResizableWindowStyle, FALSE);
		return rect;
	}

	std::wstring ToWideString(const std::string& text)
	{
		if (text.empty())
		{
			return std::wstring();
		}

		const int requiredLength = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
		if (requiredLength <= 0)
		{
			return std::wstring(text.begin(), text.end());
		}

		std::wstring wideText(static_cast<size_t>(requiredLength) - 1, L'\0');
		MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, &wideText[0], requiredLength);
		return wideText;
	}

	WindowFrame::SceneSnapshotCallback& GetSceneSnapshotCallbackStorage()
	{
		static WindowFrame::SceneSnapshotCallback callback;
		return callback;
	}
}

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

void WindowFrame::SetSceneSnapshotCallback(SceneSnapshotCallback callback)
{
	GetSceneSnapshotCallbackStorage() = std::move(callback);
}

void WindowFrame::ClearSceneSnapshotCallback()
{
	GetSceneSnapshotCallbackStorage() = SceneSnapshotCallback();
}

// imgui_impl_win32.cpp에 구현된 메시지 처리 함수를 미리 선언한다.
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
		if (MainFrame::GetInstance() != nullptr)
		{
			MainFrame::GetInstance()->RequestResize(static_cast<UINT>(LOWORD(lParam)), static_cast<UINT>(HIWORD(lParam)));
		}
		return 0;
	case WM_PAINT:
		ValidateRect(hWnd, NULL);
		return 0;
	case WM_DESTROY:
		m_Pthis->m_scene->Release();
		PostQuitMessage(0);
		return 0;
	}
	return(DefWindowProcW(hWnd, iMessage, wParam, lParam));
}

void WindowFrame::SetScene(Scene* scene)
{
	if (m_scene)
	{
		m_scene->Release();
		ObjectManager::GetInstance()->Clear();
		ResourceManager::GetInstance()->ReleaseAllResources();
		delete m_scene;
	}
	m_scene = scene;
	m_scene->Init();
	const std::string requestedSceneDataName = m_requestedSceneDataName;
	const bool hasRequestedSceneDataName = !requestedSceneDataName.empty();
	const std::string sceneName = hasRequestedSceneDataName ? requestedSceneDataName : m_scene->GetSceneName();
	m_requestedSceneDataName.clear();
	m_currentSceneName = sceneName;
	const bool useSceneData = m_scene->ShouldUseSceneData();
	const bool sceneDataExists = useSceneData && SceneDataManager::Exists(sceneName);
	bool sceneDataLoaded = false;
	if (sceneDataExists)
	{
		sceneDataLoaded = SceneDataManager::LoadSceneData(sceneName);
		if (sceneDataLoaded)
		{
			std::cout << "SceneData loaded: " << sceneName << std::endl;
		}
		else
		{
			std::cout << "SceneData load fallback: " << sceneName << std::endl;
		}
	}

	if (useSceneData && hasRequestedSceneDataName && m_type == RenderType::Game)
	{
		if (!sceneDataExists)
		{
			const std::wstring message = std::wstring(L"SceneData 파일을 찾지 못했습니다: ") + ToWideString(sceneName);
			MessageBoxW(m_hWnd, message.c_str(), L"SceneData Load Error", MB_OK | MB_ICONERROR);
			PostMessageA(m_hWnd, WM_CLOSE, 0, 0);
			return;
		}

		if (!sceneDataLoaded)
		{
			const std::wstring message = std::wstring(L"SceneData 로드에 실패했습니다: ") + ToWideString(sceneName);
			MessageBoxW(m_hWnd, message.c_str(), L"SceneData Load Error", MB_OK | MB_ICONERROR);
			PostMessageA(m_hWnd, WM_CLOSE, 0, 0);
			return;
		}
	}

	if (!sceneDataLoaded)
	{
		m_scene->BuildInitialSceneObjects();
	}
	ObjectManager::GetInstance()->FlushPendingObjects();
	m_scene->Start();

	if (m_type == RenderType::Edit && (sceneDataLoaded || m_scene->ShouldSaveInitialSceneData()))
	{
		SceneSnapshotCallback& sceneSnapshotCallback = GetSceneSnapshotCallbackStorage();
		if (sceneSnapshotCallback)
		{
			sceneSnapshotCallback(sceneName);
		}
	}
}

void WindowFrame::Initialize(RenderType type)
{
	m_type = type;
	BuildWindow();
	if (type == RenderType::Game)
	{
		const RECT startupRect = BuildStartupWindowRect();
		MoveWindow(
			m_Pthis->m_hWnd,
			100,
			100,
			startupRect.right - startupRect.left,
			startupRect.bottom - startupRect.top,
			TRUE);
	}
	SetFocus(m_Pthis->m_hWnd);
}

void WindowFrame::BuildWindow()
{
	static constexpr const wchar_t* kWindowTitle = L"최원준";

	WNDCLASSEXW WndClass =
	{
	sizeof(WNDCLASSEXW), CS_CLASSDC, WndProc, 0L, 0L,
	GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
	L"MyWindow", NULL
	};

	RegisterClassExW(&WndClass);
	const RECT startupRect = BuildStartupWindowRect();

	m_Pthis->m_hWnd =
		CreateWindowW(WndClass.lpszClassName, kWindowTitle,
			kResizableWindowStyle,
			100, 100,
			startupRect.right - startupRect.left,
			startupRect.bottom - startupRect.top,
			NULL, (HMENU)NULL, m_Pthis->m_Instance, NULL);

	SetWindowTextW(m_Pthis->m_hWnd, kWindowTitle);

	ShowWindow(m_Pthis->m_hWnd, SW_SHOWDEFAULT);
	UpdateWindow(m_Pthis->m_hWnd);

}

void WindowFrame::Run(const MSG* Message)
{
	TranslateMessage(Message);
	DispatchMessageW(Message);
}
