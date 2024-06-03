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
	bool m_isFocus = true;
	WindowFrame() {}
	~WindowFrame() {}
public:
	static void Create(HINSTANCE hInstance);
	static WindowFrame* GetInstance();
	static void Destroy();

	//콜백함수는 static이어야 한다.
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

	void SetScene(Scene* scene);
	void Initialize();
	void BuildWindow();
	void Run(const MSG* Message);
	//Buffer* GetBuffer() { return this->m_buffer; }
	HINSTANCE GetHInst() { return this->m_Instance; }
	HWND GetHWND() { return this->m_hWnd; }
	bool IsFocus() { return m_isFocus; }
};