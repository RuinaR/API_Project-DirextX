#pragma once
#include "Scene.h"
#include "Timer.h"
#include "BoxCollider.h"

enum class RenderType
{
	Edit,
	Game
};

enum class EditorPlaybackState
{
	Paused,
	Playing
};

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
	b2World* m_pWorld = nullptr;
	CollisionListener m_cListener;
private:

	HWND m_hWnd = nullptr;
	LPDIRECT3D9 m_pD3D = nullptr;
	LPDIRECT3DDEVICE9 m_pd3dDevice = nullptr;
	D3DPRESENT_PARAMETERS m_d3dpp = {};

	D3DXMATRIX m_matWorld;
	D3DXMATRIX m_matProj;
	ID3DXFont* m_pFont = nullptr;
	bool m_released = false;
	bool m_deviceResourcesInvalidated = false;

	int m_width;
	int m_height;
	bool m_pendingResize = false;
	UINT m_pendingResizeWidth = 0;
	UINT m_pendingResizeHeight = 0;

	RenderType m_type;

private:
	MSG m_message;
	double m_targetFrameTime;
	double m_frameDeltaTime = 0.0;
	int32 m_velocityIterations = 8;
	int32 m_positionIterations = 3;
	EditorPlaybackState m_editorPlaybackState = EditorPlaybackState::Paused;
	bool m_editorStepRequested = false;

	list<std::function<void()>> m_listBtnEvent;
public:
	static void Create(HINSTANCE hInstance);
	static MainFrame* GetInstance();
	static void Destroy();
	Timer& Timer();
	double DeltaTime();
	void Initialize(int targetFPS, Scene* scene, RenderType type);
	//int Run();
	//void Set();
	bool Update();
	ID3DXFont* GetFont();
	LPDIRECT3DDEVICE9 GetDevice();
	const D3DXMATRIX& GetProjectionMatrix() const;
	int GetViewportWidth() const;
	int GetViewportHeight() const;
	b2World* GetBox2dWorld();
	RenderType GetRenderType() const { return m_type; }
	void RequestResize(UINT width, UINT height);
	bool HandleResize(UINT width, UINT height);
	void ProcessMouseInput();
	void ApplyCameraProjection();
	void SetEditorPlaying(bool playing);
	void RequestEditorStep();
	bool IsEditorPlaying() const;
	bool IsEditorPaused() const;
	bool IsEditorStepRequested() const;

	void AddBtnEvent(std::function<void()> p_event);
private:
	void InvalidateDeviceResources();
	void RestoreDeviceResources();
	void ApplyDeviceState();
	bool EnsureDeviceReady();
	void Release();
};
