#pragma once
#include "Scene.h"
#include "Timer.h"
class MainFrame
{
private:
	static MainFrame* m_Pthis;
	Scene* m_scene = nullptr;
	MainFrame() {}
	~MainFrame() {}
	Timer m_timer;
	int m_targetFPS = 60;

private:

	HWND m_hWnd;
	LPDIRECT3D9 m_pD3D;
	LPDIRECT3DDEVICE9 m_pd3dDevice;

	D3DXMATRIX m_matWorld;
	D3DXMATRIX m_matProj;
	ID3DXFont* m_pFont;

	int m_width;
	int m_height;
public:
	static void Create(HINSTANCE hInstance);
	static MainFrame* GetInstance();
	static void Destroy();
	Timer& Timer();
	double DeltaTime();
	void Initialize(int targetFPS, Scene* scene);
	int Run();
	ID3DXFont* GetFont();
	LPDIRECT3DDEVICE9 GetDevice();
private:
	void Release();
};