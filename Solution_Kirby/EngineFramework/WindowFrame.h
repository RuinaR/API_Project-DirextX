#pragma once
#include "Buffer.h"
#include "Scene.h"
class WindowFrame
{
private:
	static WindowFrame* m_Pthis;

	HWND m_hWnd = NULL;
	HINSTANCE m_Instance = NULL;
	//Buffer* m_buffer = nullptr;
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

	//콜백함수는 static이어야 한다.
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

	void SetScene(Scene* scene);
	void Initialize(RenderType type);
	void BuildWindow();
	void Run(const MSG* Message);
	//Buffer* GetBuffer() { return this->m_buffer; }
	HINSTANCE GetHInst() { return this->m_Instance; }
	HWND GetHWND() { return this->m_hWnd; }
	RenderType GetRenderType() const { return m_type; }
	const char* GetCurrentSceneName() const { return m_currentSceneName.c_str(); }
	void SetCurrentSceneName(const std::string& sceneName) { m_currentSceneName = sceneName; }
	void SetRequestedSceneDataName(const std::string& sceneName) { m_requestedSceneDataName = sceneName; }
};
