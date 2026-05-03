#pragma once
#include <functional>
#include "EngineFrameworkAPI.h"
#include "Scene.h"

class ENGINEFRAMEWORK_API WindowFrame
{
public:
	using SceneSnapshotCallback = std::function<void(const std::string&)>;

private:
	static WindowFrame* m_Pthis;

	HWND m_hWnd = NULL;
	HINSTANCE m_Instance = NULL;
	// 예전에 쓰던 Buffer 포인터 자리다. 지금은 사용하지 않는다.
	Scene* m_scene = nullptr;
	std::string m_currentSceneName;
	std::string m_requestedSceneDataName;
	RenderType m_type = RenderType::Game;
	WindowFrame() {}
	~WindowFrame() {}

public:
	static void Create(HINSTANCE hInstance);
	static WindowFrame* GetInstance();
	static void Destroy();
	static void SetSceneSnapshotCallback(SceneSnapshotCallback callback);
	static void ClearSceneSnapshotCallback();

	// Win32 윈도우 프로시저는 static 함수 형태여야 한다.
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

	void SetScene(Scene* scene);
	void Initialize(RenderType type);
	void BuildWindow();
	void Run(const MSG* Message);
	// 예전에 쓰던 Buffer getter 자리다. 지금은 사용하지 않는다.
	HINSTANCE GetHInst() { return this->m_Instance; }
	HWND GetHWND() { return this->m_hWnd; }
	RenderType GetRenderType() const { return m_type; }
	const char* GetCurrentSceneName() const { return m_currentSceneName.c_str(); }
	void SetCurrentSceneName(const std::string& sceneName) { m_currentSceneName = sceneName; }
	void SetRequestedSceneDataName(const std::string& sceneName) { m_requestedSceneDataName = sceneName; }
};
