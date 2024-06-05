#pragma once
#include "Scene.h"
#include "Timer.h"
#include "BoxCollider.h"

class CollisionListener : public b2ContactListener
{
public:
	void BeginContact(b2Contact* contact) override;
	void EndContact(b2Contact* contact) override;
	void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;
	void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;
};

class MainFrame
{
private:
	static MainFrame* m_Pthis;
	Scene* m_scene = nullptr;
	MainFrame() {}
	~MainFrame() {}
	Timer m_timer;
	int m_targetFPS = 60;

	b2Vec2 m_gravity = { 0.0f, -10.0f };
	b2World* m_pWorld;
	CollisionListener m_cListener;
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
	b2World* GetBox2dWorld();
private:
	void Release();
};